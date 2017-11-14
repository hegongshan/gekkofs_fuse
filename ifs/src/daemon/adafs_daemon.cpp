
#include <daemon/adafs_daemon.hpp>
#include <db/db_util.hpp>
#include <rpc/rpc_types.hpp>
#include <rpc/rpc_defs.hpp>
#include <preload/ipc_types.hpp>

void init_environment() {
    // Initialize rocksdb
    auto err = init_rocksdb();
    assert(err);
    // init margo
    err = init_rpc_server();
    assert(err);
    err = init_ipc_server();
    assert(err);
    // TODO set metadata configurations. these have to go into a user configurable file that is parsed here
    ADAFS_DATA->atime_state(MDATA_USE_ATIME);
    ADAFS_DATA->mtime_state(MDATA_USE_MTIME);
    ADAFS_DATA->ctime_state(MDATA_USE_CTIME);
    ADAFS_DATA->uid_state(MDATA_USE_UID);
    ADAFS_DATA->gid_state(MDATA_USE_GID);
    ADAFS_DATA->inode_no_state(MDATA_USE_INODE_NO);
    ADAFS_DATA->link_cnt_state(MDATA_USE_LINK_CNT);
    ADAFS_DATA->blocks_state(MDATA_USE_BLOCKS);
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
    ADAFS_DATA->spdlogger()->info("About to finalize the margo server");
    margo_finalize(RPC_DATA->server_rpc_mid());
    margo_finalize(RPC_DATA->server_ipc_mid());
    ADAFS_DATA->spdlogger()->info("Success.");
    ADAFS_DATA->spdlogger()->info("All services shut down.");
}

bool init_ipc_server() {
    auto protocol_port = "na+sm://"s;
    hg_addr_t addr_self;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];


    ADAFS_DATA->spdlogger()->info("Initializing Margo IPC server...");
    // Start Margo (this will also initialize Argobots and Mercury internally)
    auto mid = margo_init(protocol_port.c_str(), MARGO_SERVER_MODE, 1, 16);

    if (mid == MARGO_INSTANCE_NULL) {
        ADAFS_DATA->spdlogger()->error("margo_init failed to initialize the Margo IPC server");
        return false;
    }
#ifdef MARGODIAG
    margo_diag_start(mid);
#endif
    // Figure out what address this server is listening on (must be freed when finished)
    auto hret = margo_addr_self(mid, &addr_self);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("margo_addr_self() Failed to retrieve server IPC address");
        margo_finalize(mid);
        return false;
    }
    // Convert the address to a cstring (with \0 terminator).
    hret = margo_addr_to_string(mid, addr_self_cstring, &addr_self_cstring_sz, addr_self);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("margo_addr_to_string Failed to convert address to cstring");
        margo_addr_free(mid, addr_self);
        margo_finalize(mid);
        return false;
    }
    margo_addr_free(mid, addr_self);

    ADAFS_DATA->spdlogger()->info("Success. Accepting IPCs on PID {}", addr_self_cstring);

    // Put context and class into RPC_data object
    RPC_DATA->server_ipc_mid(mid);

    // register RPCs
    register_server_rpcs(mid);

    return true;
}

bool init_rpc_server() {
    auto protocol_port = RPC_PROTOCOL + "://localhost:"s + to_string(RPCPORT);
    hg_addr_t addr_self;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];
    ADAFS_DATA->spdlogger()->info("Initializing Margo RPC server...");
    // Start Margo (this will also initialize Argobots and Mercury internally)
    auto mid = margo_init(protocol_port.c_str(), MARGO_SERVER_MODE, 1, 16);
    if (mid == MARGO_INSTANCE_NULL) {
        ADAFS_DATA->spdlogger()->error("margo_init failed to initialize the Margo RPC server");
        return false;
    }
#ifdef MARGODIAG
    margo_diag_start(mid);
#endif
    // Figure out what address this server is listening on (must be freed when finished)
    auto hret = margo_addr_self(mid, &addr_self);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("margo_addr_self() Failed to retrieve server RPC address");
        margo_finalize(mid);
        return false;
    }
    // Convert the address to a cstring (with \0 terminator).
    hret = margo_addr_to_string(mid, addr_self_cstring, &addr_self_cstring_sz, addr_self);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("margo_addr_to_string Failed to convert address to cstring");
        margo_addr_free(mid, addr_self);
        margo_finalize(mid);
        return false;
    }
    margo_addr_free(mid, addr_self);


    ADAFS_DATA->spdlogger()->info("Success. Accepting RPCs on address {}", addr_self_cstring);

    // Put context and class into RPC_data object
    RPC_DATA->server_rpc_mid(mid);

    // register RPCs
    register_server_rpcs(mid);

    return true;
}

/**
 * Register the rpcs for the server. There is no need to store rpc ids for the server
 * @param hg_class
 */
void register_server_rpcs(margo_instance_id mid) {
    if (RPC_DATA->server_ipc_mid() == mid)
        MARGO_REGISTER(mid, "ipc_srv_fs_config", ipc_config_in_t, ipc_config_out_t, ipc_srv_fs_config);
    MARGO_REGISTER(mid, "rpc_minimal", rpc_minimal_in_t, rpc_minimal_out_t, rpc_minimal);
    MARGO_REGISTER(mid, "rpc_srv_open", rpc_open_in_t, rpc_err_out_t, rpc_srv_open);
    MARGO_REGISTER(mid, "rpc_srv_stat", rpc_stat_in_t, rpc_stat_out_t, rpc_srv_stat);
    MARGO_REGISTER(mid, "rpc_srv_unlink", rpc_unlink_in_t, rpc_err_out_t, rpc_srv_unlink);
    MARGO_REGISTER(mid, "rpc_srv_update_metadentry", rpc_update_metadentry_in_t, rpc_err_out_t,
                   rpc_srv_update_metadentry);
    MARGO_REGISTER(mid, "rpc_srv_update_metadentry_size", rpc_update_metadentry_size_in_t,
                   rpc_update_metadentry_size_out_t, rpc_srv_update_metadentry_size);
    MARGO_REGISTER(mid, "rpc_srv_write_data", rpc_write_data_in_t, rpc_data_out_t, rpc_srv_write_data);
    MARGO_REGISTER(mid, "rpc_srv_read_data", rpc_read_data_in_t, rpc_data_out_t, rpc_srv_read_data);
}