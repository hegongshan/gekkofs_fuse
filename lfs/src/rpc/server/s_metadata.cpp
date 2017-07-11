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
    // get the metadata
    Metadata md{};
    get_metadata(md, in.inode);
    out.atime = static_cast<uint64_t>(md.atime());
    out.mtime = static_cast<uint64_t>(md.mtime());
    out.ctime = static_cast<uint64_t>(md.ctime());
    out.mode = static_cast<uint32_t>(md.mode());
    out.uid = static_cast<uint32_t>(md.uid());
    out.gid = static_cast<uint32_t>(md.gid());
    out.nlink = static_cast<uint64_t>(md.link_count());
    out.size = static_cast<uint64_t>(md.size());
    out.blocks = static_cast<uint64_t>(md.blocks());

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