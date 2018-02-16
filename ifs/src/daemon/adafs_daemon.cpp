
#include <daemon/adafs_daemon.hpp>
#include <db/db_util.hpp>
#include <rpc/rpc_types.hpp>
#include <rpc/rpc_defs.hpp>
#include <preload/ipc_types.hpp>
#include <adafs_ops/metadentry.hpp>

bool init_environment() {
    // Initialize rocksdb
    if (!init_rocksdb()) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to initialize RocksDB.", __func__);
        return false;
    }
    // init margo
    if (!init_rpc_server()) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to initialize Margo RPC server.", __func__);
        return false;
    }
    if (!init_ipc_server()) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to initialize Margo IPC server.", __func__);
        return false;
    }
    // Register daemon to system
    if (!register_daemon_proc()) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to register the daemon process to the system.", __func__);
        return false;
    }
    // TODO set metadata configurations. these have to go into a user configurable file that is parsed here
    ADAFS_DATA->atime_state(MDATA_USE_ATIME);
    ADAFS_DATA->mtime_state(MDATA_USE_MTIME);
    ADAFS_DATA->ctime_state(MDATA_USE_CTIME);
    ADAFS_DATA->uid_state(MDATA_USE_UID);
    ADAFS_DATA->gid_state(MDATA_USE_GID);
    ADAFS_DATA->inode_no_state(MDATA_USE_INODE_NO);
    ADAFS_DATA->link_cnt_state(MDATA_USE_LINK_CNT);
    ADAFS_DATA->blocks_state(MDATA_USE_BLOCKS);
    // Create metadentry for root directory
    if (create_metadentry(ADAFS_DATA->mountdir(), S_IFDIR | 777) != 0) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to write root metadentry to KV store.", __func__);
        return false;
    }
    ADAFS_DATA->spdlogger()->info("Startup successful. Daemon is ready.");
    return true;
}

/**
 * Destroys the margo, argobots, and mercury environments
 */
void destroy_enviroment() {
#ifdef MARGODIAG
    cout << "\n####################\n\nMargo IPC server stats: " << endl;
    margo_diag_dump(RPC_DATA->server_ipc_mid(), "-", 0);
    cout << "\n####################\n\nMargo RPC server stats: " << endl;
    margo_diag_dump(RPC_DATA->server_rpc_mid(), "-", 0);
#endif
    if (!deregister_daemon_proc())
        ADAFS_DATA->spdlogger()->warn("{}() Unable to clean up auxiliary files", __func__);
    else
        ADAFS_DATA->spdlogger()->debug("{}() Cleaning auxiliary files successful", __func__);
    // The shutdown order is important because the RPC server is started first, it has to be stopped last due to Argobots
    if (RPC_DATA->server_ipc_mid() != nullptr) {
        margo_finalize(RPC_DATA->server_ipc_mid());
        ADAFS_DATA->spdlogger()->info("{}() Margo IPC server shut down successful", __func__);
    }
    if (RPC_DATA->server_rpc_mid() != nullptr) {
        margo_finalize(RPC_DATA->server_rpc_mid());
        ADAFS_DATA->spdlogger()->info("{}() Margo RPC server shut down successful", __func__);
    }
    ADAFS_DATA->spdlogger()->info("All services shut down. ADA-FS shutdown complete.");
}

bool init_ipc_server() {
    auto protocol_port = "na+sm://"s;
    hg_addr_t addr_self;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];


    ADAFS_DATA->spdlogger()->debug("{}() Initializing Margo IPC server...", __func__);
    // Start Margo (this will also initialize Argobots and Mercury internally)
    auto mid = margo_init(protocol_port.c_str(), MARGO_SERVER_MODE, 1, IPC_HANDLER_THREADS);

    if (mid == MARGO_INSTANCE_NULL) {
        ADAFS_DATA->spdlogger()->error("{}() margo_init() failed to initialize the Margo IPC server", __func__);
        return false;
    }
#ifdef MARGODIAG
    margo_diag_start(mid);
#endif
    // Figure out what address this server is listening on (must be freed when finished)
    auto hret = margo_addr_self(mid, &addr_self);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() margo_addr_self() Failed to retrieve server IPC address", __func__);
        margo_finalize(mid);
        return false;
    }
    // Convert the address to a cstring (with \0 terminator).
    hret = margo_addr_to_string(mid, addr_self_cstring, &addr_self_cstring_sz, addr_self);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() margo_addr_to_string() Failed to convert address to cstring", __func__);
        margo_addr_free(mid, addr_self);
        margo_finalize(mid);
        return false;
    }
    margo_addr_free(mid, addr_self);

    ADAFS_DATA->spdlogger()->info("{}() Margo IPC server initialized. Accepting IPCs on PID {}", __func__,
                                  addr_self_cstring);

    // Put context and class into RPC_data object
    RPC_DATA->server_ipc_mid(mid);

    // register RPCs
    register_server_rpcs(mid);

    return true;
}

bool init_rpc_server() {
    auto protocol_port = RPC_PROTOCOL + "://localhost:"s + to_string(RPC_PORT);
    hg_addr_t addr_self;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];
    ADAFS_DATA->spdlogger()->debug("{}() Initializing Margo RPC server...", __func__);
    // Start Margo (this will also initialize Argobots and Mercury internally)
    auto mid = margo_init(protocol_port.c_str(), MARGO_SERVER_MODE, 1, RPC_HANDLER_THREADS);
    if (mid == MARGO_INSTANCE_NULL) {
        ADAFS_DATA->spdlogger()->error("{}() margo_init failed to initialize the Margo RPC server", __func__);
        return false;
    }
#ifdef MARGODIAG
    margo_diag_start(mid);
#endif
    // Figure out what address this server is listening on (must be freed when finished)
    auto hret = margo_addr_self(mid, &addr_self);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() margo_addr_self() Failed to retrieve server RPC address", __func__);
        margo_finalize(mid);
        return false;
    }
    // Convert the address to a cstring (with \0 terminator).
    hret = margo_addr_to_string(mid, addr_self_cstring, &addr_self_cstring_sz, addr_self);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() margo_addr_to_string Failed to convert address to cstring", __func__);
        margo_addr_free(mid, addr_self);
        margo_finalize(mid);
        return false;
    }
    margo_addr_free(mid, addr_self);


    ADAFS_DATA->spdlogger()->info("{}() Margo RPC server initialized. Accepting RPCs on address {}", __func__,
                                  addr_self_cstring);

    // Put context and class into RPC_data object
    RPC_DATA->server_rpc_mid(mid);

    // register RPCs
    register_server_rpcs(mid);

    return true;
}

/**
 * Registers RPC handlers to Margo instance
 * @param hg_class
 */
void register_server_rpcs(margo_instance_id mid) {
    if (RPC_DATA->server_ipc_mid() == mid)
        MARGO_REGISTER(mid, hg_tag::fs_config, ipc_config_in_t, ipc_config_out_t, ipc_srv_fs_config);
    MARGO_REGISTER(mid, hg_tag::minimal, rpc_minimal_in_t, rpc_minimal_out_t, rpc_minimal);
    MARGO_REGISTER(mid, hg_tag::create, rpc_mk_node_in_t, rpc_err_out_t, rpc_srv_mk_node);
    MARGO_REGISTER(mid, hg_tag::access, rpc_access_in_t, rpc_err_out_t, rpc_srv_access);
    MARGO_REGISTER(mid, hg_tag::stat, rpc_path_only_in_t, rpc_stat_out_t, rpc_srv_stat);
    MARGO_REGISTER(mid, hg_tag::remove, rpc_rm_node_in_t, rpc_err_out_t, rpc_srv_rm_node);
    MARGO_REGISTER(mid, hg_tag::update_metadentry, rpc_update_metadentry_in_t, rpc_err_out_t,
                   rpc_srv_update_metadentry);
    MARGO_REGISTER(mid, hg_tag::get_metadentry_size, rpc_path_only_in_t, rpc_get_metadentry_size_out_t,
                   rpc_srv_get_metadentry_size);
    MARGO_REGISTER(mid, hg_tag::update_metadentry_size, rpc_update_metadentry_size_in_t,
                   rpc_update_metadentry_size_out_t, rpc_srv_update_metadentry_size);
    MARGO_REGISTER(mid, hg_tag::write_data, rpc_write_data_in_t, rpc_data_out_t, rpc_srv_write_data);
    MARGO_REGISTER(mid, hg_tag::read_data, rpc_read_data_in_t, rpc_data_out_t, rpc_srv_read_data);
}

/**
 * Returns the path where daemon process writes information for the running clients
 * @return string
 */
string daemon_register_path() {
    return (DAEMON_AUX_PATH + "/daemon_"s + to_string(getpid()) + ".run"s);
}

/**
 * Registers the daemon process to the system.
 * This will create a file with additional information for clients started on the same node.
 * @return
 */
bool register_daemon_proc() {
    auto ret = false;
    auto daemon_aux_path = DAEMON_AUX_PATH;
    if (!bfs::exists(daemon_aux_path) && !bfs::create_directories(daemon_aux_path)) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to create adafs auxiliary directory in {}", __func__,
                                       daemon_aux_path);
        return false;
    }
    ofstream ofs(daemon_register_path().c_str(), ::ofstream::trunc);
    if (ofs) {
        ofs << ADAFS_DATA->mountdir();
        ret = true;
    }
    if (ofs.bad()) {
        perror("Error opening file to register daemon process");
        ADAFS_DATA->spdlogger()->error("{}() Error opening file to register daemon process", __func__);
        return false;
    }
    ofs.close();
    return ret;
}

bool deregister_daemon_proc() {
    return bfs::remove(daemon_register_path());
}