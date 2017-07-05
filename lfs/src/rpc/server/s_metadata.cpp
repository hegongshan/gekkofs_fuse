//
// Created by evie on 6/22/17.
//
#include "../rpc_types.hpp"
#include "../rpc_defs.hpp"
#include "../../adafs_ops/mdata_ops.hpp"

static hg_return_t rpc_minimal(hg_handle_t handle) {
    rpc_minimal_in_t in;
    rpc_minimal_out_t out;
    const struct hg_info* hgi;
    // Get input
    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);

    ADAFS_DATA->spdlogger()->info("Got simple RPC with input {}", in.input);
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

static hg_return_t rpc_srv_create(hg_handle_t handle) {
    rpc_create_in_t in;
    rpc_create_out_t out;
    const struct hg_info* hgi;

    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->info("Got create RPC with filename {}", in.filename);

    hgi = HG_Get_info(handle);

    auto mid = margo_hg_class_to_instance(hgi->hg_class);
    fuse_entry_param fep{};
    create_node(fep, in.parent_inode, in.filename, in.uid, in.gid, in.mode);
    out.new_inode = fep.ino;
    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.new_inode);
    auto hret = margo_respond(mid, handle, &out);
    assert(hret == HG_SUCCESS);

    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}
DEFINE_MARGO_RPC_HANDLER(rpc_srv_create)

static hg_return_t rpc_srv_attr(hg_handle_t handle) {
    rpc_get_attr_in_t in;
    rpc_get_attr_out_t out;
    const struct hg_info* hgi;
    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->info("Got get attr RPC with inode {}", in.inode);
    hgi = HG_Get_info(handle);
    auto mid = margo_hg_class_to_instance(hgi->hg_class);

    struct stat attr{};
    get_attr(attr, in.inode);
    out.atime = attr.st_atim.tv_sec;
    out.mtime = attr.st_mtim.tv_sec;
    out.ctime = attr.st_ctim.tv_sec;
    out.mode = attr.st_mode;
    out.uid = attr.st_uid;
    out.gid = attr.st_gid;
    out.nlink = attr.st_nlink;
    out.size = attr.st_size;
    out.blocks = attr.st_blocks;
    ADAFS_DATA->spdlogger()->debug("Sending output mode {}", out.mode);
    auto hret = margo_respond(mid, handle, &out);
    assert(hret == HG_SUCCESS);

    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_attr)