//
// Created by evie on 6/21/17.
//

#include "rpc_util.hpp"

using namespace std;

bool init_argobots() {
    ADAFS_DATA->spdlogger()->info("Initializing Argobots ...");

    // We need no arguments to init
    auto argo_err = ABT_init(0, nullptr);
    if (argo_err != 0) {
        ADAFS_DATA->spdlogger()->info("[ERR]: ABT_init() Failed to init Argobots (client)");
        return false;
    }
    // Set primary execution stream to idle without polling. Normally xstreams cannot sleep. This is what ABT_snoozer does
    argo_err = ABT_snoozer_xstream_self_set();
    if (argo_err != 0) {
        ADAFS_DATA->spdlogger()->info("[ERR]: ABT_snoozer_xstream_self_set()  (client)");
        return false;
    }
    ADAFS_DATA->spdlogger()->info("Success.");
    return true;
}

bool destroy_argobots() {
    ABT_finalize();
}

bool init_rpc_server() {
    auto protocol_port = "cci+tcp://3344"s;

    hg_addr_t addr_self;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];

    // Mercury class and context pointer that go into RPC_data class
    hg_class_t* hg_class;
    hg_context_t* hg_context;

    ADAFS_DATA->spdlogger()->info("Initializing Mercury server ...");
    /* MERCURY PART */
    // Init Mercury layer (must be finalized when finished)
    hg_class = HG_Init(protocol_port.c_str(), HG_TRUE);
    if (hg_class == nullptr) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Init() Failed to init Mercury server layer");
        return false;
    }
    // Create a new Mercury context (must be destroyed when finished)
    hg_context = HG_Context_create(hg_class);
    if (hg_context == nullptr) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Context_create() Failed to create Mercury server context");
        HG_Finalize(hg_class);
        return false;
    }
    // Below is just for logging purposes
    // Figure out what address this server is listening on (must be freed when finished)
    auto hg_ret = HG_Addr_self(hg_class, &addr_self);
    if (hg_ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Addr_self() Failed to retrieve server address");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    // Convert the address to a cstring (with \0 terminator).
    hg_ret = HG_Addr_to_string(hg_class, addr_self_cstring, &addr_self_cstring_sz, addr_self);
    if (hg_ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Addr_to_string Failed to convert address to cstring");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        HG_Addr_free(hg_class, addr_self);
        return false;
    }
    HG_Addr_free(hg_class, addr_self);

    ADAFS_DATA->spdlogger()->info("Success. Accepting RPCs on address {}", addr_self_cstring);

    /* MARGO PART */
    ADAFS_DATA->spdlogger()->info("Initializing Margo server...");
    // Start Margo TODO set first two parameters later.
    auto m_id = margo_init(0, 0, hg_context);
    if (m_id == MARGO_INSTANCE_NULL) {
        ADAFS_DATA->spdlogger()->info("[ERR]: margo_init failed to initialize the Margo server");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    ADAFS_DATA->spdlogger()->info("Success.");

    // register RPCs TODO
//    register_server_rpcs_test(hg_class);


    // Put context and class into RPC_data object
    RPC_DATA->server_hg_class(hg_class);
    RPC_DATA->server_hg_context(hg_context);

    return true;
}

void destroy_rpc_server() {
    HG_Context_destroy(RPC_DATA->server_hg_context());
    HG_Finalize(RPC_DATA->server_hg_class());
}

bool init_rpc_client() {
    auto protocol_port = "cci+tcp"s;
    ADAFS_DATA->spdlogger()->info("Initializing Mercury client ...");
    /* MERCURY PART */
    // Init Mercury layer (must be finalized when finished)
    hg_class_t* hg_class;
    hg_context_t* hg_context;
    hg_class = HG_Init(protocol_port.c_str(), HG_FALSE);
    if (hg_class == nullptr) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Init() Failed to init Mercury client layer");
        return false;
    }
    // Create a new Mercury context (must be destroyed when finished)
    hg_context = HG_Context_create(hg_class);
    if (hg_context == nullptr) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Context_create() Failed to create Mercury client context");
        HG_Finalize(hg_class);
        return false;
    }
    ADAFS_DATA->spdlogger()->info("Success.");

    /* MARGO PART */
    ADAFS_DATA->spdlogger()->info("Initializing Margo client ...");
    // Start Margo TODO set first two parameters later.
    auto m_id = margo_init(0, 0, hg_context);
    if (m_id == MARGO_INSTANCE_NULL) {
        ADAFS_DATA->spdlogger()->info("[ERR]: margo_init failed to initialize the Margo client");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    ADAFS_DATA->spdlogger()->info("Success.");

    // TODO register RPCs and populate the ids of some enum struct which I can call to search the rpc_id for a fuse function
//    auto rpc_id = register_client_rpcs_test(hg_class);

    // Put context and class into RPC_data object
    RPC_DATA->client_hg_class(hg_class);
    RPC_DATA->client_hg_context(hg_context);

    return true;
}

void destroy_rpc_client() {
    HG_Context_destroy(RPC_DATA->client_hg_context());
    HG_Finalize(RPC_DATA->client_hg_class());
}
