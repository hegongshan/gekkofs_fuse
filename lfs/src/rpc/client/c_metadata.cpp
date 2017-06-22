//
// Created by evie on 6/22/17.
//

#include "c_metadata.hpp"

void send_minimal_rpc() {
    hg_handle_t handle;
    rpc_minimal_in_t in;
    rpc_minimal_out_t out;
    hg_addr_t svr_addr = HG_ADDR_NULL;

    ADAFS_DATA->spdlogger()->info("minimal RPC is running...");

    margo_addr_lookup(RPC_DATA->client_mid(), "cci+tcp://localhost:3344", &svr_addr);

    /* create handle */
    auto ret = HG_Create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_minimal_id(), &handle);
    assert(ret == HG_SUCCESS);

    /* Send rpc. Note that we are also transmitting the bulk handle in the
     * input struct.  It was set above.
     */
    in.input = 42;
    margo_forward(RPC_DATA->client_mid(), handle, &in);

    /* decode response */
    ret = HG_Get_output(handle, &out);
    assert(ret == HG_SUCCESS);

    ADAFS_DATA->spdlogger()->info("Got response ret: {}", out.output);

    /* clean up resources consumed by this rpc */
    HG_Free_output(handle, &out);
    HG_Destroy(handle);

    ADAFS_DATA->spdlogger()->info("minimal RPC is done.");
}