//
// Created by evie on 6/14/17.
//

#include "rpc_util.hpp"
#include "rpcs.hpp"

using namespace std;

bool init_margo_server() {
    auto protocol_port = "cci+tcp://3344"s;

    hg_addr_t addr_self;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];

    ADAFS_DATA->spdlogger()->info("Mercury: Initializing Mercury ...");
    /* MERCURY PART */
    // Init Mercury layer (must be finalized when finished)
    hg_class_t* hg_class;
    hg_class = HG_Init(protocol_port.c_str(), HG_TRUE);
    if (hg_class == nullptr) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Init() Failed to init Mercury layer");
        return false;
    }
    // Create a new Mercury context (must be destroyed when finished)
    auto hg_context = HG_Context_create(hg_class);
    if (hg_context == nullptr) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Context_create() Failed to create Mercury context");
        HG_Finalize(hg_class);
        return false;
    }
    // Figure out what address this server is listening on (must be freed when finished)
    auto hg_ret = HG_Addr_self(hg_class, &addr_self);
    if (hg_ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Addr_self() Failed to retrieve server address");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    // Convert the address to a cstring (with \0 terminator)
    hg_ret = HG_Addr_to_string(hg_class, addr_self_cstring, &addr_self_cstring_sz, addr_self);
    if (hg_ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Addr_to_string Failed to convert address to cstring");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        HG_Addr_free(hg_class, addr_self);
        return false;
    }
    HG_Addr_free(hg_class, addr_self);

    ADAFS_DATA->spdlogger()->info("Mercury: Accepting RPCs on address {}", addr_self_cstring);

    ADAFS_DATA->spdlogger()->info("Initializing Argobots ...");
    /* ARGOBOTS PART */
    // We need no arguments to init
    auto argo_ret = ABT_init(0, nullptr);
    if (argo_ret != 0) {
        ADAFS_DATA->spdlogger()->info("[ERR]: ABT_init() Failed to init Argobots");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    // Set primary ES to idle without polling XXX (Not sure yet what that does)
    argo_ret = ABT_snoozer_xstream_self_set();
    if (argo_ret != 0) {
        ADAFS_DATA->spdlogger()->info("[ERR]: ABT_snoozer_xstream_self_set()");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }

    ADAFS_DATA->spdlogger()->info("Initializing Margo ...");
    /* MARGO PART */
    // Start Margo TODO set first two parameters later.
    auto margo_id = margo_init(0, 0, hg_context);
    assert(margo_id);
    ADAFS_DATA->spdlogger()->info("Margo successfully initialized");

    // register RPCs
    register_rpcs(hg_class);

    margo_wait_for_finalize(margo_id);


    ADAFS_DATA->spdlogger()->info("Shutting down RPC framework");
    ADAFS_DATA->spdlogger()->info("Finalizing Margo ...");
    margo_finalize(margo_id);
    ADAFS_DATA->spdlogger()->info("Finalizing Argobots ...");
    ABT_finalize();
    ADAFS_DATA->spdlogger()->info("Mercury: Destroying hg_context and finalizing hg_class ...");
    HG_Context_destroy(hg_context);
    HG_Finalize(hg_class);
    ADAFS_DATA->spdlogger()->info("RPC Framework shut down");
    return true;
}

void register_rpcs(hg_class_t* hg_class) {
    MERCURY_REGISTER(hg_class, "my_rpc", my_rpc_in_t, my_rpc_out_t,
                     my_rpc_ult_handler);
    MERCURY_REGISTER(hg_class, "my_shutdown_rpc", void, void,
                     my_rpc_shutdown_ult_handler);
}

bool destroy_margo_server(hg_class_t* hg_class, hg_context_t* hg_context) {

    HG_Context_destroy(hg_context);
    HG_Finalize(hg_class);
    return true;
}