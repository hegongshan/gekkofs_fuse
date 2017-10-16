//
// Created by evie on 8/31/17.
//

#include <daemon/adafs_daemon.hpp>
#include <db/db_util.hpp>
#include <rpc/rpc_types.hpp>
#include <rpc/rpc_defs.hpp>
#include <preload/ipc_types.hpp>

void daemon_loop(void* arg) {
    ADAFS_DATA->spdlogger()->info("Starting application loop ...");
    while (true) {
        ADAFS_DATA->spdlogger()->info("sleeping");
        sleep(10);
        /* TODO for Nafiseh
         * Connect to the IPC socket with the looping thread and listed for messages from the preload lib
         * When new message is received spawn a new thread that will trigger the operation and respond to preload lib
         * Ensure that messages from the lib are not lost. XXX
         */

        // connect to the ipc socket and a separate thread retrieves the message from the preload lib. in
        ADAFS_DATA->spdlogger()->info("done sleeping. exiting ...");
        break;
    }
}

void run_daemon() {
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_thread loop_thread;

    auto argo_ret = ABT_xstream_self(
            &xstream); // get the current execution stream (1 core) to use (started with ABT_init())
    if (argo_ret != 0) {
        ADAFS_DATA->spdlogger()->error("Error getting the execution stream when starting the daemon.");
        return;
    }
    argo_ret = ABT_xstream_get_main_pools(xstream, 1, &pool); // get the thread pool
    if (argo_ret != 0) {
        ADAFS_DATA->spdlogger()->error("Error getting the thread pool when starting the daemon.");
        return;
    }
    argo_ret = ABT_thread_create(pool, daemon_loop, nullptr, ABT_THREAD_ATTR_NULL, &loop_thread);
    if (argo_ret != 0) {
        ADAFS_DATA->spdlogger()->error("Error creating loop thread");
        return;
    }

    // wait for the daemon to be closed and free the loop thread
    ABT_thread_yield_to(loop_thread);
    argo_ret = ABT_thread_join(loop_thread);
    if (argo_ret != 0) {
        ADAFS_DATA->spdlogger()->error("Error joining loop thread");
        return;
    }
    argo_ret = ABT_thread_free(&loop_thread);
    if (argo_ret != 0) {
        ADAFS_DATA->spdlogger()->error("Error freeing loop thread.");
        return;
    }

}

void init_environment() {
    // Initialize rocksdb
    auto err = init_rocksdb();
    assert(err);
    err = init_argobots();
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
    ADAFS_DATA->spdlogger()->info("About to finalize the margo server and client");
    margo_finalize(RPC_DATA->client_mid());
    margo_finalize(RPC_DATA->server_rpc_mid());
    margo_finalize(RPC_DATA->server_ipc_mid());
    ADAFS_DATA->spdlogger()->info("Success.");

    destroy_argobots();

    ADAFS_DATA->spdlogger()->info("About to destroy the mercury context");
    HG_Context_destroy(RPC_DATA->client_hg_context());
    HG_Context_destroy(RPC_DATA->server_rpc_hg_context());
    HG_Context_destroy(RPC_DATA->server_ipc_hg_context());
    ADAFS_DATA->spdlogger()->info("Success.");
    ADAFS_DATA->spdlogger()->info("About to destroy the mercury class");
    HG_Finalize(RPC_DATA->client_hg_class());
    HG_Finalize(RPC_DATA->server_rpc_hg_class());
    HG_Finalize(RPC_DATA->server_ipc_hg_class());
    ADAFS_DATA->spdlogger()->info("Success.");
    ADAFS_DATA->spdlogger()->info("All services shut down.");
}

/**
 * Initializes the Argobots environment
 * @return
 */
bool init_argobots() {
    ADAFS_DATA->spdlogger()->info("Initializing Argobots ...");

    // We need no arguments to init
    auto argo_err = ABT_init(0, nullptr);
    if (argo_err != 0) {
        ADAFS_DATA->spdlogger()->error("ABT_init() Failed to init Argobots (client)");
        return false;
    }
    // Set primary execution stream to idle without polling. Normally xstreams cannot sleep. This is what ABT_snoozer does
    argo_err = ABT_snoozer_xstream_self_set();
    if (argo_err != 0) {
        ADAFS_DATA->spdlogger()->error("ABT_snoozer_xstream_self_set()  (client)");
        return false;
    }
    ADAFS_DATA->spdlogger()->info("Success.");
    return true;
}

/**
 * Shuts down Argobots
 */
void destroy_argobots() {
    ADAFS_DATA->spdlogger()->info("About to shut Argobots down"s);
    auto ret = ABT_finalize();
    if (ret == ABT_SUCCESS) {
        ADAFS_DATA->spdlogger()->info("Argobots successfully shutdown.");
    } else {
        ADAFS_DATA->spdlogger()->error("Argobots shutdown FAILED with err code {}", ret);
    }
}

bool init_ipc_server() {
    auto protocol_port = "na+sm://"s;
    hg_addr_t addr_self;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];

    // Mercury class and context pointer that go into RPC_data class
    hg_class_t* hg_class;
    hg_context_t* hg_context;

    ADAFS_DATA->spdlogger()->info("Initializing Mercury IPC server ...");
    /* MERCURY PART */
    // Init Mercury layer (must be finalized when finished)
    hg_class = HG_Init(protocol_port.c_str(), HG_TRUE);
    if (hg_class == nullptr) {
        ADAFS_DATA->spdlogger()->error("HG_Init() Failed to init Mercury IPC server layer");
        return false;
    }
    // Create a new Mercury context (must be destroyed when finished)
    hg_context = HG_Context_create(hg_class);
    if (hg_context == nullptr) {
        ADAFS_DATA->spdlogger()->error("HG_Context_create() Failed to create Mercury IPC server context");
        HG_Finalize(hg_class);
        return false;
    }
    // Below is just for logging purposes
    // Figure out what address this server is listening on (must be freed when finished)
    auto hg_ret = HG_Addr_self(hg_class, &addr_self);
    if (hg_ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("HG_Addr_self() Failed to retrieve IPC server address");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    // Convert the address to a cstring (with \0 terminator).
    hg_ret = HG_Addr_to_string(hg_class, addr_self_cstring, &addr_self_cstring_sz, addr_self);
    if (hg_ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("HG_Addr_to_string Failed to convert address to cstring");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        HG_Addr_free(hg_class, addr_self);
        return false;
    }
    HG_Addr_free(hg_class, addr_self);

    ADAFS_DATA->spdlogger()->info("Success. Accepting IPCs on PID {}", addr_self_cstring);

    /* MARGO PART */
    ADAFS_DATA->spdlogger()->info("Initializing Margo IPC server...");
    // Start Margo
    auto mid = margo_init(1, 16, hg_context);
    if (mid == MARGO_INSTANCE_NULL) {
        ADAFS_DATA->spdlogger()->error("margo_init failed to initialize the Margo IPC server");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    ADAFS_DATA->spdlogger()->info("Success.");

    // Put context and class into RPC_data object
    RPC_DATA->server_ipc_hg_class(hg_class);
    RPC_DATA->server_ipc_hg_context(hg_context);
    RPC_DATA->server_ipc_mid(mid);

    // register RPCs
    register_server_ipcs();

    return true;
}

bool init_rpc_server() {
    auto protocol_port = RPC_PROTOCOL + "://localhost:"s + to_string(RPCPORT);
    hg_addr_t addr_self;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];

    // Mercury class and context pointer that go into RPC_data class
    hg_class_t* hg_class;
    hg_context_t* hg_context;

    ADAFS_DATA->spdlogger()->info("Initializing Mercury RPC server ...");
    /* MERCURY PART */
    // Init Mercury layer (must be finalized when finished)
    hg_class = HG_Init(protocol_port.c_str(), HG_TRUE);
    if (hg_class == nullptr) {
        ADAFS_DATA->spdlogger()->error("HG_Init() Failed to init Mercury RPC server layer");
        return false;
    }
    // Create a new Mercury context (must be destroyed when finished)
    hg_context = HG_Context_create(hg_class);
    if (hg_context == nullptr) {
        ADAFS_DATA->spdlogger()->error("HG_Context_create() Failed to create Mercury RPC server context");
        HG_Finalize(hg_class);
        return false;
    }
    // Below is just for logging purposes
    // Figure out what address this server is listening on (must be freed when finished)
    auto hg_ret = HG_Addr_self(hg_class, &addr_self);
    if (hg_ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("HG_Addr_self() Failed to retrieve server RPC address");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    // Convert the address to a cstring (with \0 terminator).
    hg_ret = HG_Addr_to_string(hg_class, addr_self_cstring, &addr_self_cstring_sz, addr_self);
    if (hg_ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("HG_Addr_to_string Failed to convert address to cstring");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        HG_Addr_free(hg_class, addr_self);
        return false;
    }
    HG_Addr_free(hg_class, addr_self);

    ADAFS_DATA->spdlogger()->info("Success. Accepting RPCs on address {}", addr_self_cstring);

    /* MARGO PART */
    ADAFS_DATA->spdlogger()->info("Initializing Margo RPC server...");
    // Start Margo
    auto mid = margo_init(1, 16, hg_context);
    if (mid == MARGO_INSTANCE_NULL) {
        ADAFS_DATA->spdlogger()->error("margo_init failed to initialize the Margo RPC server");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    ADAFS_DATA->spdlogger()->info("Success.");

    // Put context and class into RPC_data object
    RPC_DATA->server_rpc_hg_class(hg_class);
    RPC_DATA->server_rpc_hg_context(hg_context);
    RPC_DATA->server_rpc_mid(mid);

    // register RPCs
    register_server_rpcs();

    return true;
}

// TODO these two can be merged as soon as ipc equals rpc (which it should)
void register_server_ipcs() {
    auto hg_class = RPC_DATA->server_ipc_hg_class();
    // preload IPCs
    MERCURY_REGISTER(hg_class, "rpc_minimal", rpc_minimal_in_t, rpc_minimal_out_t, rpc_minimal_handler);
    MERCURY_REGISTER(hg_class, "ipc_srv_fs_config", ipc_config_in_t, ipc_config_out_t, ipc_srv_fs_config_handler);
    MERCURY_REGISTER(hg_class, "ipc_srv_open", ipc_open_in_t, ipc_err_out_t, ipc_srv_open_handler);
    MERCURY_REGISTER(hg_class, "ipc_srv_stat", ipc_stat_in_t, ipc_stat_out_t, ipc_srv_stat_handler);
    MERCURY_REGISTER(hg_class, "ipc_srv_unlink", ipc_unlink_in_t, ipc_err_out_t, ipc_srv_unlink_handler);
    MERCURY_REGISTER(hg_class, "rpc_srv_update_metadentry", rpc_update_metadentry_in_t, ipc_err_out_t,
                     rpc_srv_update_metadentry_handler);
    MERCURY_REGISTER(hg_class, "rpc_srv_update_metadentry_size", rpc_update_metadentry_size_in_t,
                     rpc_update_metadentry_size_out_t, rpc_srv_update_metadentry_size_handler);
    MERCURY_REGISTER(hg_class, "rpc_srv_write_data", rpc_write_data_in_t, rpc_data_out_t, rpc_srv_write_data_handler);
    MERCURY_REGISTER(hg_class, "rpc_srv_read_data", rpc_read_data_in_t, rpc_data_out_t, rpc_srv_read_data_handler);
}

/**
 * Register the rpcs for the server. There is no need to store rpc ids for the server
 * @param hg_class
 */
void register_server_rpcs() {
    auto hg_class = RPC_DATA->server_rpc_hg_class();
    MERCURY_REGISTER(hg_class, "rpc_minimal", rpc_minimal_in_t, rpc_minimal_out_t, rpc_minimal_handler);
    MERCURY_REGISTER(hg_class, "rpc_srv_create_node", rpc_create_node_in_t, rpc_err_out_t, rpc_srv_create_node_handler);
    MERCURY_REGISTER(hg_class, "rpc_srv_attr", rpc_get_attr_in_t, rpc_get_attr_out_t, rpc_srv_attr_handler);
    MERCURY_REGISTER(hg_class, "rpc_srv_remove_node", rpc_remove_node_in_t, rpc_err_out_t, rpc_srv_remove_node_handler);
    MERCURY_REGISTER(hg_class, "rpc_srv_update_metadentry", rpc_update_metadentry_in_t, ipc_err_out_t,
                     rpc_srv_update_metadentry_handler);
    MERCURY_REGISTER(hg_class, "rpc_srv_update_metadentry_size", rpc_update_metadentry_size_in_t,
                     rpc_update_metadentry_size_out_t, rpc_srv_update_metadentry_size_handler);
    MERCURY_REGISTER(hg_class, "rpc_srv_write_data", rpc_write_data_in_t, rpc_data_out_t, rpc_srv_write_data_handler);
    MERCURY_REGISTER(hg_class, "rpc_srv_read_data", rpc_read_data_in_t, rpc_data_out_t, rpc_srv_read_data_handler);
}