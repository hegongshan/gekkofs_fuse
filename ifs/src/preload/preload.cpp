#include <dlfcn.h>

#include <preload/preload.hpp>
#include <global/rpc/ipc_types.hpp>
#include <preload/margo_ipc.hpp>
#include <preload/rpc/ld_rpc_data_ws.hpp>
#include <preload/passthrough.hpp>

enum class Margo_mode {
    RPC, IPC
};

// atomic bool to check if auxiliary files from daemon are loaded
std::atomic<bool> is_aux_loaded_(false);
// thread to initialize the whole margo shazaam only once per process
static pthread_once_t init_env_thread = PTHREAD_ONCE_INIT;

// external variables that are initialized here
// IPC IDs
hg_id_t ipc_minimal_id;
hg_id_t ipc_config_id;
hg_id_t ipc_mk_node_id;
hg_id_t ipc_access_id;
hg_id_t ipc_stat_id;
hg_id_t ipc_rm_node_id;
hg_id_t ipc_update_metadentry_id;
hg_id_t ipc_get_metadentry_size_id;
hg_id_t ipc_update_metadentry_size_id;
hg_id_t ipc_write_data_id;
hg_id_t ipc_read_data_id;
// RPC IDs
hg_id_t rpc_minimal_id;
hg_id_t rpc_mk_node_id;
hg_id_t rpc_access_id;
hg_id_t rpc_stat_id;
hg_id_t rpc_rm_node_id;
hg_id_t rpc_update_metadentry_id;
hg_id_t rpc_get_metadentry_size_id;
hg_id_t rpc_update_metadentry_size_id;
hg_id_t rpc_write_data_id;
hg_id_t rpc_read_data_id;
// Margo instances
margo_instance_id ld_margo_ipc_id;
margo_instance_id ld_margo_rpc_id;
// rpc address cache
KVCache rpc_address_cache{32768, 4096}; // XXX Set values are not based on anything...
// local daemon IPC address
hg_addr_t daemon_svr_addr = HG_ADDR_NULL;
// IO RPC driver
ABT_pool io_pool;
std::vector<ABT_xstream> io_streams;

/**
 * Initializes the Argobots environment
 * @return
 */
bool init_ld_argobots() {
    ld_logger->debug("{}() Initializing Argobots ...", __func__);

    // We need no arguments to init
    auto argo_err = ABT_init(0, nullptr);
    if (argo_err != 0) {
        ld_logger->error("{}() ABT_init() Failed to init Argobots (client)", __func__);
        return false;
    }
    // Set primary execution stream to idle without polling. Normally xstreams cannot sleep. This is what ABT_snoozer does
    argo_err = ABT_snoozer_xstream_self_set();
    if (argo_err != 0) {
        ld_logger->error("{}() ABT_snoozer_xstream_self_set()  (client)", __func__);
        return false;
    }
    /*
     * Single producer (progress function) and multiple consumers are causing an excess memory consumption
     * in some Argobots version. It does only show if an ES with a pool is created.
     * Although this is probably not an issue on the consumer process, we set reduce the Argobots stack size here,
     * just in case we change the client process in the future.
     * See for reference: https://xgitlab.cels.anl.gov/sds/margo/issues/40
     */
    putenv(const_cast<char*>("ABT_MEM_MAX_NUM_STACKS=8"));
    // Creating pool for driving IO RPCs
    vector<ABT_xstream> io_streams_tmp(IO_LIBRARY_THREADS);
    argo_err = ABT_snoozer_xstream_create(IO_LIBRARY_THREADS, &io_pool, io_streams_tmp.data());
    if (argo_err != ABT_SUCCESS) {
        ld_logger->error("{}() ABT_snoozer_xstream_create()  (client)", __func__);
        return false;
    }
    io_streams = io_streams_tmp;
    ld_logger->debug("{}() Argobots initialization successful.", __func__);
    return true;
}

/**
 * Registers a margo instance with all used RPC, differentiating between IPC and RPC client
 * Note that the rpc tags are redundant for rpc and ipc ids
 * @param mid
 * @param mode
 */
void register_client_rpcs(margo_instance_id mid, Margo_mode mode) {
    if (mode == Margo_mode::IPC) {
        // IPC IDs
        ipc_config_id = MARGO_REGISTER(mid, hg_tag::fs_config, ipc_config_in_t, ipc_config_out_t,
                                       NULL);
        ipc_minimal_id = MARGO_REGISTER(mid, hg_tag::minimal, rpc_minimal_in_t, rpc_minimal_out_t, NULL);
        ipc_mk_node_id = MARGO_REGISTER(mid, hg_tag::create, rpc_mk_node_in_t, rpc_err_out_t, NULL);
        ipc_access_id = MARGO_REGISTER(mid, hg_tag::access, rpc_access_in_t, rpc_err_out_t, NULL);
        ipc_stat_id = MARGO_REGISTER(mid, hg_tag::stat, rpc_path_only_in_t, rpc_stat_out_t, NULL);
        ipc_rm_node_id = MARGO_REGISTER(mid, hg_tag::remove, rpc_rm_node_in_t,
                                        rpc_err_out_t, NULL);
        ipc_update_metadentry_id = MARGO_REGISTER(mid, hg_tag::update_metadentry, rpc_update_metadentry_in_t,
                                                  rpc_err_out_t, NULL);
        ipc_get_metadentry_size_id = MARGO_REGISTER(mid, hg_tag::get_metadentry_size, rpc_path_only_in_t,
                                                    rpc_get_metadentry_size_out_t, NULL);
        ipc_update_metadentry_size_id = MARGO_REGISTER(mid, hg_tag::update_metadentry_size,
                                                       rpc_update_metadentry_size_in_t,
                                                       rpc_update_metadentry_size_out_t,
                                                       NULL);
        ipc_write_data_id = MARGO_REGISTER(mid, hg_tag::write_data, rpc_write_data_in_t, rpc_data_out_t,
                                           NULL);
        ipc_read_data_id = MARGO_REGISTER(mid, hg_tag::read_data, rpc_read_data_in_t, rpc_data_out_t,
                                          NULL);
    } else {
        // RPC IDs
        rpc_minimal_id = MARGO_REGISTER(mid, hg_tag::minimal, rpc_minimal_in_t, rpc_minimal_out_t, NULL);
        rpc_mk_node_id = MARGO_REGISTER(mid, hg_tag::create, rpc_mk_node_in_t, rpc_err_out_t, NULL);
        rpc_access_id = MARGO_REGISTER(mid, hg_tag::access, rpc_access_in_t, rpc_err_out_t, NULL);
        rpc_stat_id = MARGO_REGISTER(mid, hg_tag::stat, rpc_path_only_in_t, rpc_stat_out_t, NULL);
        rpc_rm_node_id = MARGO_REGISTER(mid, hg_tag::remove, rpc_rm_node_in_t,
                                        rpc_err_out_t, NULL);
        rpc_update_metadentry_id = MARGO_REGISTER(mid, hg_tag::update_metadentry, rpc_update_metadentry_in_t,
                                                  rpc_err_out_t, NULL);
        rpc_get_metadentry_size_id = MARGO_REGISTER(mid, hg_tag::get_metadentry_size, rpc_path_only_in_t,
                                                    rpc_get_metadentry_size_out_t, NULL);
        rpc_update_metadentry_size_id = MARGO_REGISTER(mid, hg_tag::update_metadentry_size,
                                                       rpc_update_metadentry_size_in_t,
                                                       rpc_update_metadentry_size_out_t,
                                                       NULL);
        rpc_write_data_id = MARGO_REGISTER(mid, hg_tag::write_data, rpc_write_data_in_t, rpc_data_out_t,
                                           NULL);
        rpc_read_data_id = MARGO_REGISTER(mid, hg_tag::read_data, rpc_read_data_in_t, rpc_data_out_t,
                                          NULL);
    }
}

/**
 * Initializes the Margo client for a given na_plugin
 * @param mode
 * @param na_plugin
 * @return
 */
bool init_margo_client(Margo_mode mode, const string na_plugin) {

    ABT_xstream xstream = ABT_XSTREAM_NULL;
    ABT_pool pool = ABT_POOL_NULL;

    // get execution stream and its main pools
    auto ret = ABT_xstream_self(&xstream);
    if (ret != ABT_SUCCESS)
        return false;
    ret = ABT_xstream_get_main_pools(xstream, 1, &pool);
    if (ret != ABT_SUCCESS) return false;
    if (mode == Margo_mode::IPC)
        ld_logger->debug("{}() Initializing Mercury IPC client ...", __func__);
    else
        ld_logger->debug("{}() Initializing Mercury RPC client ...", __func__);
    /* MERCURY PART */
    // Init Mercury layer (must be finalized when finished)
    hg_class_t* hg_class;
    hg_context_t* hg_context;
    // Note: localhost should not be required and actually doesn't do anything. But it is required for OFI for Mercury to start
    hg_class = HG_Init((na_plugin + "://localhost"s).c_str(), HG_FALSE);
    if (hg_class == nullptr) {
        ld_logger->error("{}() HG_Init() Failed to init Mercury client layer", __func__);
        return false;
    }
    // Create a new Mercury context (must be destroyed when finished)
    hg_context = HG_Context_create(hg_class);
    if (hg_context == nullptr) {
        ld_logger->error("{}() HG_Context_create() Failed to create Mercury client context", __func__);
        HG_Finalize(hg_class);
        return false;
    }
    ld_logger->debug("{}() Mercury initialized.", __func__);

    /* MARGO PART */
    if (mode == Margo_mode::IPC)
        ld_logger->debug("{}() Initializing Margo IPC client ...", __func__);
    else
        ld_logger->debug("{}() Initializing Margo RPC client ...", __func__);
    // margo will run in the context of thread
    auto mid = margo_init_pool(pool, pool, hg_context);
    if (mid == MARGO_INSTANCE_NULL) {
        ld_logger->error("{}() margo_init_pool failed to initialize the Margo client", __func__);
        return false;
    }
    ld_logger->debug("{}() Margo initialized.", __func__);

    if (mode == Margo_mode::IPC) {
        ld_margo_ipc_id = mid;
        auto adafs_daemon_pid = getProcIdByName("adafs_daemon"s);
        if (adafs_daemon_pid == -1) {
            ld_logger->error("{}() ADA-FS daemon not started. Exiting ...", __func__);
            return false;
        }
        ld_logger->debug("{}() ADA-FS daemon with PID {} found.", __func__, adafs_daemon_pid);

        string sm_addr_str = "na+sm://"s + to_string(adafs_daemon_pid) + "/0";
        margo_addr_lookup(ld_margo_ipc_id, sm_addr_str.c_str(), &daemon_svr_addr);
    } else
        ld_margo_rpc_id = mid;

    register_client_rpcs(mid, mode);
#ifdef MARGODIAG
    margo_diag_start(mid);
#endif
    return true;
}

/**
 * Returns atomic bool, if Margo is running
 * @return
 */
bool ld_is_aux_loaded() {
    return is_aux_loaded_;
}

/**
 * This function is only called in the preload constructor and initializes Argobots and Margo clients
 */
void init_ld_environment_() {
    if (!init_ld_argobots()) {
        ld_logger->error("{}() Unable to initialize Argobots.", __func__);
        exit(EXIT_FAILURE);
    }
    if (!init_margo_client(Margo_mode::IPC, "na+sm"s)) {
        ld_logger->error("{}() Unable to initialize Margo IPC client.", __func__);
        exit(EXIT_FAILURE);
    }
    if (!ipc_send_get_fs_config()) {
        ld_logger->error("{}() Unable to fetch file system configurations from daemon process through IPC.", __func__);
        exit(EXIT_FAILURE);
    }
    if (!init_margo_client(Margo_mode::RPC, RPC_PROTOCOL)) {
        ld_logger->error("{}() Unable to initialize Margo RPC client.", __func__);
        exit(EXIT_FAILURE);
    }
    if (!read_system_hostfile()) {
        ld_logger->error("{}() Unable to read system hostfile /etc/hosts for address mapping.", __func__);
        exit(EXIT_FAILURE);
    }
    ld_logger->info("{}() Environment initialization successful.", __func__);
}

void init_ld_env_if_needed() {
    pthread_once(&init_env_thread, init_ld_environment_);
}


/**
 * Called initially ONCE when preload library is used with the LD_PRELOAD environment variable
 */
void init_preload() {
    init_passthrough_if_needed();
    // The logger is initialized in init_passthrough. So we cannot log before that.
    ld_logger->info("{}() enter", __func__); // XXX For client hang investigation
    if (!get_daemon_auxiliaries() || fs_config->mountdir.empty()) {
        perror("Error while getting daemon auxiliaries");
        ld_logger->error("{}() while getting daemon auxiliaries", __func__);
        exit(EXIT_FAILURE);
    } else {
        ld_logger->info("{}() mountdir \"{}\" loaded from daemon auxiliaries", __func__, fs_config->mountdir);
        is_aux_loaded_ = true;
    }
    ld_logger->info("{}() exit", __func__); // XXX For client hang investigation
}

/**
 * Called last when preload library is used with the LD_PRELOAD environment variable
 */
void destroy_preload() {
    auto services_used = (ld_margo_ipc_id != nullptr || ld_margo_rpc_id != nullptr);
#ifdef MARGODIAG
    if (ld_margo_ipc_id != nullptr) {
        cout << "\n####################\n\nMargo IPC client stats: " << endl;
        margo_diag_dump(ld_margo_ipc_id, "-", 0);
    }
    if (ld_margo_rpc_id != nullptr) {
        cout << "\n####################\n\nMargo RPC client stats: " << endl;
        margo_diag_dump(ld_margo_rpc_id, "-", 0);
    }
#endif
    for (auto& io_stream : io_streams) {
        ABT_xstream_join(io_stream);
        ABT_xstream_free(&io_stream);
    }
    ld_logger->debug("{}() Freeing IO execution streams successful", __func__);
    // Shut down RPC client if used
    if (ld_margo_rpc_id != nullptr) {
        // free all rpc addresses in LRU map and finalize margo rpc
        ld_logger->debug("{}() Freeing Margo RPC svr addresses ...", __func__);
        auto free_all_addr = [&](const KVCache::node_type& n) {
            if (margo_addr_free(ld_margo_rpc_id, n.value) != HG_SUCCESS) {
                ld_logger->warn("{}() Unable to free RPC client's svr address: {}.", __func__, n.key);
            }
        };
        rpc_address_cache.cwalk(free_all_addr);
        ld_logger->debug("{}() About to finalize the margo RPC client. Actually not doing it XXX", __func__);
//        margo_finalize(ld_margo_rpc_id);
        ld_logger->debug("{}() Shut down Margo RPC client successful", __func__);
    }
    // Shut down IPC client if used
    if (ld_margo_ipc_id != nullptr) {
        ld_logger->debug("{}() Freeing Margo IPC daemon svr address ...", __func__);
        if (margo_addr_free(ld_margo_ipc_id, daemon_svr_addr) != HG_SUCCESS)
            ld_logger->warn("{}() Unable to free IPC client's daemon svr address.", __func__);
        ld_logger->debug("{}() About to finalize the margo IPC client. Actually not doing it XXX", __func__);
//        margo_finalize(ld_margo_ipc_id);
        ld_logger->debug("{}() Shut down Margo IPC client successful", __func__);
    }
    if (services_used)
        ld_logger->info("All services shut down. Client shutdown complete.");
    else
        ld_logger->debug("{}() No services in preload library used. Nothing to shut down.", __func__);
}