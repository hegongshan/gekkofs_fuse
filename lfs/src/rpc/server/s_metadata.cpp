//
// Created by evie on 6/22/17.
//
#include "../rpc_types.hpp"
#include "../rpc_defs.hpp"

static void rpc_minimal(hg_handle_t handle) {
    rpc_minimal_in_t in;
    rpc_minimal_out_t out;
    // Get input
    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);

    ADAFS_DATA->spdlogger()->info("Got simple RPC with input {}", in.input);
    // Get hg_info handle
    auto hgi = HG_Get_info(handle);
    // extract margo id from hg_info (needed to know where to send response)
    auto mid = margo_hg_class_to_instance(hgi->hg_class);

    // Create output and send it
    out.output = in.input * 2;
    ADAFS_DATA->spdlogger()->info("Sending output {}", out.output);
    auto hret = margo_respond(mid, handle, &out);
    assert(hret == HG_SUCCESS);
    // Destroy handle when finished
    HG_Destroy(handle);

}

DEFINE_MARGO_RPC_HANDLER(rpc_minimal)