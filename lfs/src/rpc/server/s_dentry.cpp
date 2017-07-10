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

static hg_return_t rpc_srv_create_dentry(hg_handle_t handle) {
    rpc_create_dentry_in_t in;
    rpc_create_dentry_out_t out;
    const struct hg_info* hgi;

    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->info("Got create dentry RPC with filename {}", in.filename);

    hgi = HG_Get_info(handle);

    auto mid = margo_hg_class_to_instance(hgi->hg_class);
    // create new inode number and then the dentry
    auto new_inode = Util::generate_inode_no();
    if (!create_dentry(in.parent_inode, new_inode, in.filename, in.mode)) {
        // if putting dentry failed, return invalid inode to indicate failure
        new_inode = INVALID_INODE;
    }
    out.inode = new_inode;
    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.inode);
    auto hret = margo_respond(mid, handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to create dentry rpc");
    }

    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_create_dentry)

static hg_return_t rpc_srv_remove_dentry(hg_handle_t handle) {
    rpc_remove_dentry_in_t in;
    rpc_remove_dentry_out_t out;
    const struct hg_info* hgi;

    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->info("Got remove dentry RPC with filename {}", in.filename);

    hgi = HG_Get_info(handle);

    auto mid = margo_hg_class_to_instance(hgi->hg_class);
    // remove dentry
    fuse_ino_t del_inode;
    int err;
    tie(err, del_inode) = remove_dentry(in.parent_inode, in.filename);
    if (err != 0)
        del_inode = INVALID_INODE;
    out.del_inode = del_inode;
    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.del_inode);
    auto hret = margo_respond(mid, handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to remove dentry rpc");
    }

    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_remove_dentry)