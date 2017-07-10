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


static hg_return_t rpc_srv_create_mdata(hg_handle_t handle) {
    rpc_create_mdata_in_t in;
    rpc_create_mdata_out_t out;

    const struct hg_info* hgi;

    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->info("Got create mdata RPC with inode {}", in.inode);

    hgi = HG_Get_info(handle);

    auto mid = margo_hg_class_to_instance(hgi->hg_class);
    // create metadata
    init_metadata(in.inode, in.uid, in.gid, in.mode);
    out.success = HG_TRUE;
    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.success);
    auto hret = margo_respond(mid, handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to create mdata rpc");
    }

    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_create_mdata)


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

static hg_return_t rpc_srv_remove_mdata(hg_handle_t handle) {
    rpc_remove_mdata_in_t in;
    rpc_remove_mdata_out_t out;

    const struct hg_info* hgi;

    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->info("Got remove mdata RPC with inode {}", in.del_inode);

    hgi = HG_Get_info(handle);

    auto mid = margo_hg_class_to_instance(hgi->hg_class);
    // delete metadata
    auto err = remove_all_metadata(static_cast<fuse_ino_t>(in.del_inode));
    if (err == 0)
        out.success = HG_TRUE;
    else
        out.success = HG_FALSE;
    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.success);
    auto hret = margo_respond(mid, handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to remove mdata rpc");
    }

    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_remove_mdata)