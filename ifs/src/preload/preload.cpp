#include <dlfcn.h>

#include <preload/preload.hpp>
#include <preload/ipc_types.hpp>
#include <preload/margo_ipc.hpp>
#include <preload/rpc/ld_rpc_data.hpp>
#include <preload/passthrough.hpp>

enum class Margo_mode {
    RPC, IPC
};

// atomic bool to check for margo initialization XXX This has to become more robus
std::atomic<bool> is_env_initialized(false);

// external variables that are initialized here
// IPC IDs
hg_id_t ipc_minimal_id;
hg_id_t ipc_config_id;
hg_id_t ipc_open_id;
hg_id_t ipc_stat_id;
hg_id_t ipc_unlink_id;
hg_id_t ipc_update_metadentry_id;
hg_id_t ipc_update_metadentry_size_id;
hg_id_t ipc_write_data_id;
hg_id_t ipc_read_data_id;
// RPC IDs
hg_id_t rpc_minimal_id;
hg_id_t rpc_open_id;
hg_id_t rpc_stat_id;
hg_id_t rpc_unlink_id;
hg_id_t rpc_update_metadentry_id;
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

/**
 * Initializes the Argobots environment
 * @return
 */
bool init_ld_argobots() {
    ld_logger->info("Initializing Argobots ...");

    // We need no arguments to init
    auto argo_err = ABT_init(0, nullptr);
    if (argo_err != 0) {
        ld_logger->info("ABT_init() Failed to init Argobots (client)");
        return false;
    }
    // Set primary execution stream to idle without polling. Normally xstreams cannot sleep. This is what ABT_snoozer does
    argo_err = ABT_snoozer_xstream_self_set();
    if (argo_err != 0) {
        ld_logger->info("ABT_snoozer_xstream_self_set()  (client)");
        return false;
    }
    ld_logger->info("Success.");
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
        ipc_config_id = MARGO_REGISTER(mid, IPC_FS_CONFIG_TAG, ipc_config_in_t, ipc_config_out_t,
                                       NULL);
        ipc_minimal_id = MARGO_REGISTER(mid, RPC_MINIMAL_TAG, rpc_minimal_in_t, rpc_minimal_out_t, NULL);
        ipc_open_id = MARGO_REGISTER(mid, RPC_OPEN_TAG, rpc_open_in_t, rpc_err_out_t, NULL);
        ipc_stat_id = MARGO_REGISTER(mid, RPC_STAT_TAG, rpc_stat_in_t, rpc_stat_out_t, NULL);
        ipc_unlink_id = MARGO_REGISTER(mid, RPC_UNLINK_TAG, rpc_unlink_in_t,
                                       rpc_err_out_t, NULL);
        ipc_update_metadentry_id = MARGO_REGISTER(mid, RPC_UPDATE_METADENTRY_TAG, rpc_update_metadentry_in_t,
                                                  rpc_err_out_t, NULL);
        ipc_update_metadentry_size_id = MARGO_REGISTER(mid, RPC_UPDATE_METADENTRY_SIZE_TAG,
                                                       rpc_update_metadentry_size_in_t,
                                                       rpc_update_metadentry_size_out_t,
                                                       NULL);
        ipc_write_data_id = MARGO_REGISTER(mid, RPC_WRITE_DATA_TAG, rpc_write_data_in_t, rpc_data_out_t,
                                           NULL);
        ipc_read_data_id = MARGO_REGISTER(mid, RPC_READ_DATA_TAG, rpc_read_data_in_t, rpc_data_out_t,
                                          NULL);
    } else {
        // RPC IDs
        rpc_minimal_id = MARGO_REGISTER(mid, RPC_MINIMAL_TAG, rpc_minimal_in_t, rpc_minimal_out_t, NULL);
        rpc_open_id = MARGO_REGISTER(mid, RPC_OPEN_TAG, rpc_open_in_t, rpc_err_out_t, NULL);
        rpc_stat_id = MARGO_REGISTER(mid, RPC_STAT_TAG, rpc_stat_in_t, rpc_stat_out_t, NULL);
        rpc_unlink_id = MARGO_REGISTER(mid, RPC_UNLINK_TAG, rpc_unlink_in_t,
                                       rpc_err_out_t, NULL);
        rpc_update_metadentry_id = MARGO_REGISTER(mid, RPC_UPDATE_METADENTRY_TAG, rpc_update_metadentry_in_t,
                                                  rpc_err_out_t, NULL);
        rpc_update_metadentry_size_id = MARGO_REGISTER(mid, RPC_UPDATE_METADENTRY_SIZE_TAG,
                                                       rpc_update_metadentry_size_in_t,
                                                       rpc_update_metadentry_size_out_t,
                                                       NULL);
        rpc_write_data_id = MARGO_REGISTER(mid, RPC_WRITE_DATA_TAG, rpc_write_data_in_t, rpc_data_out_t,
                                           NULL);
        rpc_read_data_id = MARGO_REGISTER(mid, RPC_READ_DATA_TAG, rpc_read_data_in_t, rpc_data_out_t,
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
        ld_logger->info("Initializing Mercury IPC client ...");
    else
        ld_logger->info("Initializing Mercury RPC client ...");
    /* MERCURY PART */
    // Init Mercury layer (must be finalized when finished)
    hg_class_t* hg_class;
    hg_context_t* hg_context;
    hg_class = HG_Init(na_plugin.c_str(), HG_FALSE);
    if (hg_class == nullptr) {
        ld_logger->info("HG_Init() Failed to init Mercury client layer");
        return false;
    }
    // Create a new Mercury context (must be destroyed when finished)
    hg_context = HG_Context_create(hg_class);
    if (hg_context == nullptr) {
        ld_logger->info("HG_Context_create() Failed to create Mercury client context");
        HG_Finalize(hg_class);
        return false;
    }
    ld_logger->info("Success.");

    /* MARGO PART */
    if (mode == Margo_mode::IPC)
        ld_logger->info("Initializing Margo IPC client ...");
    else
        ld_logger->info("Initializing Margo RPC client ...");
    // margo will run in the context of thread
    auto mid = margo_init_pool(pool, pool, hg_context);
    if (mid == MARGO_INSTANCE_NULL) {
        ld_logger->error("margo_init_pool failed to initialize the Margo client");
        return false;
    }
    ld_logger->info("Success.");

    if (mode == Margo_mode::IPC) {
        ld_margo_ipc_id = mid;
        auto adafs_daemon_pid = getProcIdByName("adafs_daemon"s);
        if (adafs_daemon_pid == -1) {
            ld_logger->error("{}() ADA-FS daemon not started. Exiting ...", __func__);
            return false;
        }
        ld_logger->info("{}() ADA-FS daemon with PID {} found.", __func__, adafs_daemon_pid);

        string sm_addr_str = "na+sm://"s + to_string(adafs_daemon_pid) + "/0";
        margo_addr_lookup(ld_margo_ipc_id, sm_addr_str.c_str(), &daemon_svr_addr);
    } else
        ld_margo_rpc_id = mid;

    register_client_rpcs(mid, mode);
#ifdef MARGODIAG
    margo_diag_start(mid);
#endif
//    for (int i = 0; i < 10; ++i) {
//        printf("Running %d iteration\n", i);
//        send_minimal_ipc(minimal_id);
//    }

    return true;
}

/**
 * Returns atomic bool, if Margo is running
 * @return
 */
bool ld_is_env_initialized() {
    return is_env_initialized;
}

/**
 * This function is only called in the preload constructor and initializes Argobots and Margo clients
 */
void init_environment() {
    // init margo client for IPC
    auto err = init_ld_argobots();
    assert(err);
    err = init_margo_client(Margo_mode::IPC, "na+sm"s);
    assert(err);
    err = ipc_send_get_fs_config(); // get fs configurations the daemon was started with.
    assert(err);
    err = init_margo_client(Margo_mode::RPC, RPC_PROTOCOL);
    assert(err);
    is_env_initialized = true;
    ld_logger->info("Environment initialized.");
}

/**
 * Called initially when preload library is used with the LD_PRELOAD environment variable
 */
void init_preload() {
    init_passthrough_if_needed();
    init_environment();
    ld_logger->info("{}() successful.", __func__);
}

/**
 * Called last when preload library is used with the LD_PRELOAD environment variable
 */
void destroy_preload() {
#ifdef MARGODIAG
    cout << "\n####################\n\nMargo IPC client stats: " << endl;
    margo_diag_dump(margo_ipc_id_, "-", 0);
    cout << "\n####################\n\nMargo RPC client stats: " << endl;
    margo_diag_dump(margo_rpc_id_, "-", 0);
#endif
    ld_logger->info("Freeing Mercury daemon addr ...");
    HG_Addr_free(margo_get_class(ld_margo_ipc_id), daemon_svr_addr);
    ld_logger->info("Finalizing Margo IPC client ...");
    auto mercury_ipc_class = margo_get_class(ld_margo_ipc_id);
    auto mercury_ipc_context = margo_get_context(ld_margo_ipc_id);
    margo_finalize(ld_margo_ipc_id);

    ld_logger->info("Freeing Mercury RPC addresses ...");
    // free all rpc addresses in LRU map and finalize margo rpc
    auto free_all_addr = [&](const KVCache::node_type& n) {
        HG_Addr_free(margo_get_class(ld_margo_rpc_id), n.value);
    };
    rpc_address_cache.cwalk(free_all_addr);
    ld_logger->info("Finalizing Margo RPC client ...");
    auto mercury_rpc_class = margo_get_class(ld_margo_rpc_id);
    auto mercury_rpc_context = margo_get_context(ld_margo_rpc_id);
    margo_finalize(ld_margo_rpc_id);

    ld_logger->info("Destroying Mercury context ...");
    HG_Context_destroy(mercury_ipc_context);
    HG_Context_destroy(mercury_rpc_context);
    ld_logger->info("Finalizing Mercury class ...");
    HG_Finalize(mercury_ipc_class);
    HG_Finalize(mercury_rpc_class);
    ld_logger->info("Preload library shut down.");

    ld_logger->info("Finalizing Argobots ...");
    ABT_finalize();
}