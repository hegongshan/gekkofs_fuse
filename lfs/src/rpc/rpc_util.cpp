//
// Created by evie on 6/14/17.
//

#include "rpc_util.hpp"
#include "rpcs.hpp"

using namespace std;

// TODO I need 1 argobots instance (i.e. ABT_pool to create threads)
// TODO I need 1 hg_class and 1 hg_context instances for the SERVER
// TODO I need 1 hg_class and 1 hg_context instances for the CLIENT
// TODO I must store all client RPC IDs somewhere to be able to send rpcs (i.e., to use margo_forward())
// TODO I need a translation for modulo operation of keys to the actual IPs


bool init_margo_server() {
    auto protocol_port = "cci+tcp://3344"s;

    hg_addr_t addr_self;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];

    ADAFS_DATA->spdlogger()->info("Mercury: Initializing Mercury server ...");
    /* MERCURY PART */
    // Init Mercury layer (must be finalized when finished)
    hg_class_t* hg_class;
    hg_class = HG_Init(protocol_port.c_str(), HG_TRUE);
    if (hg_class == nullptr) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Init() Failed to init Mercury server layer");
        return false;
    }
    // Create a new Mercury context (must be destroyed when finished)
    auto hg_context = HG_Context_create(hg_class);
    if (hg_context == nullptr) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Context_create() Failed to create Mercury server context");
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
    register_server_rpcs(hg_class);

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

void register_server_rpcs(hg_class_t* hg_class) {
    MERCURY_REGISTER(hg_class, "my_rpc", my_rpc_in_t, my_rpc_out_t, my_rpc_ult_handler);
    MERCURY_REGISTER(hg_class, "my_shutdown_rpc", void, void, my_rpc_shutdown_ult_handler);
    MERCURY_REGISTER(hg_class, "my_rpc_minimal", my_rpc_minimal_in_t, my_rpc_minimal_out_t, my_rpc_minimal_handler);
}

bool init_margo_client() {
    auto protocol_port = "cci+tcp"s;
    ADAFS_DATA->spdlogger()->info("Mercury: Initializing Mercury client ...");
    /* MERCURY PART */
    // Init Mercury layer (must be finalized when finished)
    hg_class_t* hg_class;
    hg_class = HG_Init(protocol_port.c_str(), HG_FALSE);
    if (hg_class == nullptr) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Init() Failed to init Mercury client layer");
        return false;
    }
    // Create a new Mercury context (must be destroyed when finished)
    auto hg_context = HG_Context_create(hg_class);
    if (hg_context == nullptr) {
        ADAFS_DATA->spdlogger()->info("[ERR]: HG_Context_create() Failed to create Mercury client context");
        HG_Finalize(hg_class);
        return false;
    }

    ADAFS_DATA->spdlogger()->info("Initializing Argobots ...");
    /* ARGOBOTS PART */

    ABT_xstream xstream;
    ABT_pool pool;
    // We need no arguments to init
    auto argo_ret = ABT_init(0, nullptr);
    if (argo_ret != 0) {
        ADAFS_DATA->spdlogger()->info("[ERR]: ABT_init() Failed to init Argobots (client)");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    // Set primary ES to idle without polling XXX (Not sure yet what that does)
    argo_ret = ABT_snoozer_xstream_self_set();
    if (argo_ret != 0) {
        ADAFS_DATA->spdlogger()->info("[ERR]: ABT_snoozer_xstream_self_set()  (client)");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    argo_ret = ABT_xstream_self(&xstream);
    if (argo_ret != 0) {
        ADAFS_DATA->spdlogger()->info("[ERR]: ABT_xstream_self()  (client)");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    argo_ret = ABT_xstream_get_main_pools(xstream, 1, &pool);
    if (argo_ret != 0) {
        ADAFS_DATA->spdlogger()->info("[ERR]: ABT_xstream_get_main_pools()  (client)");
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
    auto rpc_id = register_client_rpcs(hg_class);

    // below should be the actual rpc calls
    run_my_rpc(margo_id, hg_class, hg_context, pool, rpc_id);

    margo_finalize(margo_id);

    ABT_finalize();
    HG_Context_destroy(hg_context);
    HG_Finalize(hg_class);


    return true;
}

void run_my_rpc(margo_instance_id& margo_id, hg_class_t* hg_class, hg_context_t* hg_context, ABT_pool& pool,
                hg_id_t rpc_id) {
    hg_addr_t svr_addr = HG_ADDR_NULL;
//    ABT_thread thread;
    hg_handle_t handle;

    my_rpc_in_t in;
    my_rpc_out_t out;

    auto protocol_port = "cci+tcp://localhost:3344"s;
    /* find addr for server */
    auto margo_ret = margo_addr_lookup(margo_id, protocol_port.c_str(), &svr_addr);
    assert(margo_ret == HG_SUCCESS);

    struct run_my_rpc_args args;
    args.mid = margo_id;
    args.svr_addr = svr_addr;
    args.val = 42;
    args.hg_class = hg_class;
    args.hg_context = hg_context;

    ADAFS_DATA->spdlogger()->info("Sending RPC...");

    margo_ret = HG_Create(args.hg_context, args.svr_addr, rpc_id, &handle);
    assert(margo_ret == HG_SUCCESS);

    const struct hg_info* hgi;
    hg_size_t size;
    void* buffer;
    size = 512;
    buffer = calloc(1, 512);
    hgi = HG_Get_info(handle);
    margo_ret = HG_Bulk_create(hgi->hg_class, 1, &buffer, &size, HG_BULK_READ_ONLY, &in.bulk_handle);


    in.input_val = args.val;
    // send rpc
    margo_forward(margo_id, handle, &in);

    // get response
    margo_ret = HG_Get_output(handle, &out);
    assert(margo_ret == HG_SUCCESS);

    ADAFS_DATA->spdlogger()->info("Got RPC response {}", out.ret);


    HG_Free_output(handle, &out);
    HG_Destroy(handle);

    HG_Addr_free(hg_class, args.svr_addr);
    ADAFS_DATA->spdlogger()->info("RPC sending done.");

    return;
}

void run_my_minimal_rpc(margo_instance_id& margo_id, hg_class_t* hg_class, hg_context_t* hg_context, hg_id_t rpc_id) {
    hg_addr_t svr_addr = HG_ADDR_NULL;
    hg_handle_t handle;

    my_rpc_minimal_in_t in;
    my_rpc_minimal_out_t out;

    auto protocol_port = "cci+tcp://localhost:3344"s;
    /* find addr for server */
    auto margo_ret = margo_addr_lookup(margo_id, protocol_port.c_str(), &svr_addr);
    assert(margo_ret == HG_SUCCESS);

    ADAFS_DATA->spdlogger()->info("Sending RPC...");

    margo_ret = HG_Create(hg_context, svr_addr, rpc_id, &handle);
    assert(margo_ret == HG_SUCCESS);


    in.input = 42;
    // send rpc
    margo_forward(margo_id, handle, &in);

    // get response
    margo_ret = HG_Get_output(handle, &out);
    assert(margo_ret == HG_SUCCESS);

    ADAFS_DATA->spdlogger()->info("Got RPC response {}", out.output);


    HG_Free_output(handle, &out);
    HG_Destroy(handle);

    HG_Addr_free(hg_class, svr_addr);
    ADAFS_DATA->spdlogger()->info("RPC sending done.");

    return;
}


hg_id_t register_client_rpcs(hg_class_t* hg_class) {
    // TODO just for testing. all rpc_ids need to be stored somewhere.
    hg_id_t rpc_id = MERCURY_REGISTER(hg_class, "my_rpc", my_rpc_in_t, my_rpc_out_t,
                                      nullptr);
    MERCURY_REGISTER(hg_class, "my_shutdown_rpc", void, void,
                     nullptr);
    MERCURY_REGISTER(hg_class, "my_rpc_minimal", my_rpc_minimal_in_t, my_rpc_minimal_out_t,
                     nullptr);
    return rpc_id;
}


bool destroy_margo_server(hg_class_t* hg_class, hg_context_t* hg_context) {

    HG_Context_destroy(hg_context);
    HG_Finalize(hg_class);
    return true;
}