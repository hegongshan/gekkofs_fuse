//
// Created by evie on 9/12/17.
//

#include <rpc/rpc_defs.hpp>
#include <rpc/rpc_utils.hpp>
#include <preload/ipc_types.hpp>

#include <daemon/fs_operations.hpp>
#include <adafs_ops/metadentry.hpp>
#include <db/db_ops.hpp>

using namespace std;

static hg_return_t ipc_srv_fs_config(hg_handle_t handle) {
    ipc_config_in_t in;
    ipc_config_out_t out;


    auto ret = margo_get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->info("Got config IPC");

    // get fs config
    out.mountdir = ADAFS_DATA->mountdir().c_str();
    out.rootdir = ADAFS_DATA->rootdir().c_str();
    out.atime_state = static_cast<hg_bool_t>(ADAFS_DATA->atime_state());
    out.mtime_state = static_cast<hg_bool_t>(ADAFS_DATA->mtime_state());
    out.ctime_state = static_cast<hg_bool_t>(ADAFS_DATA->ctime_state());
    out.uid_state = static_cast<hg_bool_t>(ADAFS_DATA->uid_state());
    out.gid_state = static_cast<hg_bool_t>(ADAFS_DATA->gid_state());
    out.inode_no_state = static_cast<hg_bool_t>(ADAFS_DATA->inode_no_state());
    out.link_cnt_state = static_cast<hg_bool_t>(ADAFS_DATA->link_cnt_state());
    out.blocks_state = static_cast<hg_bool_t>(ADAFS_DATA->blocks_state());
    out.uid = getuid();
    out.gid = getgid();
    out.hosts_raw = static_cast<hg_const_string_t>(ADAFS_DATA->hosts_raw().c_str());
    out.host_id = static_cast<hg_uint64_t>(ADAFS_DATA->host_id());
    out.host_size = static_cast<hg_uint64_t>(ADAFS_DATA->host_size());
    ADAFS_DATA->spdlogger()->info("Sending output configs back to library");
    auto hret = margo_respond(handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to open ipc");
    }

    out.mountdir = nullptr;
    out.rootdir = nullptr;
    out.hosts_raw = nullptr;

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_free_output(handle, &out);
    margo_destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(ipc_srv_fs_config)

static hg_return_t ipc_srv_open(hg_handle_t handle) {
    ipc_open_in_t in;
    ipc_err_out_t out;


    auto ret = margo_get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got open IPC with path {}", in.path);

    // do open TODO handle da flags
    string path = in.path;
    out.err = create_metadentry(in.path, in.mode);

    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.err);
    auto hret = margo_respond(handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to open ipc");
    }

    in.path = nullptr;

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_free_output(handle, &out);
    margo_destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(ipc_srv_open)

static hg_return_t ipc_srv_stat(hg_handle_t handle) {
    ipc_stat_in_t in;
    ipc_stat_out_t out;


    auto ret = margo_get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got stat IPC with path {}", in.path);

    // do open
    string val;
    auto err = db_get_metadentry(in.path, val);
    if (err) {
        out.err = 0;
        out.db_val = val.c_str();
    } else {
        out.err = 1;
    }
    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.err);
    auto hret = margo_respond(handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to stat ipc");
    }

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_free_output(handle, &out);
    margo_destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(ipc_srv_stat)

static hg_return_t ipc_srv_unlink(hg_handle_t handle) {
    ipc_unlink_in_t in;
    ipc_err_out_t out;

    auto ret = margo_get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got unlink IPC with path {}", in.path);

    // do unlink
    out.err = remove_node(in.path);
    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.err);
    auto hret = margo_respond(handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to unlink ipc");
    }

    in.path = nullptr;

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_free_output(handle, &out);
    margo_destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(ipc_srv_unlink)