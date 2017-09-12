//
// Created by evie on 9/12/17.
//

#include <rpc/rpc_defs.hpp>
#include <preload/ipc_types.hpp>
#include <daemon/fs_operations.hpp>

using namespace std;

static hg_return_t ipc_srv_open(hg_handle_t handle) {
    const struct hg_info* hgi;
    ipc_open_in_t in;
    ipc_res_out_t out;


    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got create node IPC with path {}", in.path);

    hgi = HG_Get_info(handle);

    auto mid = margo_hg_class_to_instance(hgi->hg_class);

    // do open
    string path = in.path;
    auto err = adafs_open(path, in.flags, in.mode);
    if (err == 0) {
        out.res = HG_TRUE;

    }
    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.res);
    auto hret = margo_respond(mid, handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to open ipc");
    }

    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(ipc_srv_open)