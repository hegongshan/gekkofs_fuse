//
// Created by evie on 9/7/17.
//

#include "rpc/rpc_types.hpp"
#include "rpc/rpc_defs.hpp"
#include "adafs_ops/metadentry.hpp"

static hg_return_t rpc_minimal(hg_handle_t handle) {
    rpc_minimal_in_t in;
    rpc_minimal_out_t out;
    const struct hg_info* hgi;
    // Get input
    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);

    ADAFS_DATA->spdlogger()->debug("Got simple RPC with input {}", in.input);
    // Get hg_info handle
    hgi = HG_Get_info(handle);
    // extract margo id from hg_info (needed to know where to send response)
    auto mid = margo_hg_class_to_instance(hgi->hg_class);

    // Create output and send it
    out.output = in.input * 2;
    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.output);
    auto hret = margo_respond(mid, handle, &out);
    assert(hret == HG_SUCCESS);
    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_minimal)

static hg_return_t rpc_srv_create_node(hg_handle_t handle) {
    const struct hg_info* hgi;
    rpc_create_node_in_t in;
    rpc_res_out_t out;


    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got create node RPC with path {}", in.path);

    hgi = HG_Get_info(handle);

    auto mid = margo_hg_class_to_instance(hgi->hg_class);

    // create metadentry
    auto err = create_metadentry(in.path, in.mode);
    if (err == 0) {
        out.res = HG_TRUE;

    }
    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.res);
    auto hret = margo_respond(mid, handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to create node rpc");
    }

    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_create_node)