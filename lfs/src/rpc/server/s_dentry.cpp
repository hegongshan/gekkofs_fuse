//
// Created by evie on 7/7/17.
//
#include "../rpc_types.hpp"
#include "../rpc_defs.hpp"
#include "../../adafs_ops/dentry_ops.hpp"

using namespace std;

static hg_return_t rpc_srv_lookup(hg_handle_t handle) {
    rpc_lookup_in_t in;
    rpc_lookup_out_t out;
    fuse_ino_t inode;
    int err;
    const struct hg_info* hgi;

    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->info("Got lookup RPC with filename {}", in.filename);

    hgi = HG_Get_info(handle);

    auto mid = margo_hg_class_to_instance(hgi->hg_class);
    tie(err, inode) = do_lookup(in.parent_inode, in.filename);

    if (err != 0) {
        inode = INVALID_INODE;
    }

    out.inode = static_cast<uint64_t>(inode);
    ADAFS_DATA->spdlogger()->debug("Sending output inode {}", out.inode);
    auto hret = margo_respond(mid, handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to lookup");
    }

    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_lookup)
