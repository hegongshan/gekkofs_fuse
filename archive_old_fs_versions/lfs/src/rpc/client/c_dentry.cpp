//
// Created by evie on 7/7/17.
//

#include "c_dentry.hpp"
#include "../rpc_types.hpp"

using namespace std;

static int max_retries = 3;

int rpc_send_lookup(const size_t recipient, const fuse_ino_t parent, const char* name, fuse_ino_t& inode) {

    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_lookup_in_t in;
    rpc_lookup_out_t out;
    auto err = 0;
    // fill in
    in.parent_inode = static_cast<uint64_t>(parent);
    in.filename = name;
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!RPC_DATA->get_addr_by_hostid(recipient, svr_addr)) {
        ADAFS_DATA->spdlogger()->error("server address not resolvable for host id {}", recipient);
        return 1;
    }
    auto ret = HG_Create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_srv_lookup_id(), &handle);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("creating handle FAILED");
        return 1;
    }
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(RPC_DATA->client_mid(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = HG_Get_output(handle, &out);

        ADAFS_DATA->spdlogger()->debug("Got response inode: {}", out.inode);
        inode = static_cast<fuse_ino_t>(out.inode);
        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        ADAFS_DATA->spdlogger()->error("RPC send_lookup(timed out)");
    }

    in.filename = nullptr;
    HG_Free_input(handle, &in);
    HG_Destroy(handle);

    if (inode == INVALID_INODE)
        err = ENOENT;
    return err;
}

int rpc_send_create_dentry(const size_t recipient, const fuse_ino_t parent, const string& name,
                           const mode_t mode, fuse_ino_t& new_inode) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_create_dentry_in_t in;
    rpc_create_dentry_out_t out;
    auto err = 0;
    // fill in
    in.parent_inode = static_cast<uint64_t>(parent);
    in.filename = name.c_str();
    in.mode = static_cast<uint32_t>(mode);
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!RPC_DATA->get_addr_by_hostid(recipient, svr_addr)) {
        ADAFS_DATA->spdlogger()->error("server address not resolvable for host id {}", recipient);
        return 1;
    }
    auto ret = HG_Create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_srv_create_dentry_id(), &handle);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("creating handle FAILED");
        return 1;
    }
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(RPC_DATA->client_mid(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = HG_Get_output(handle, &out);

        ADAFS_DATA->spdlogger()->debug("Got response inode: {}", out.inode);
        new_inode = static_cast<fuse_ino_t>(out.inode);

        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        ADAFS_DATA->spdlogger()->error("RPC send_create_dentry (timed out)");
    }

    in.filename = nullptr; // XXX temporary. If this is not done free input crashes because of invalid pointer?!
    HG_Free_input(handle, &in);
    HG_Destroy(handle);
    if (new_inode == INVALID_INODE)
        err = 1;

    return err;
}

int rpc_send_remove_dentry(const size_t recipient, const fuse_ino_t parent, const string& name, fuse_ino_t& del_inode) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_remove_dentry_in_t in;
    rpc_remove_dentry_out_t out;
    auto err = 0;
    // fill in
    in.parent_inode = static_cast<uint64_t>(parent);
    in.filename = name.c_str();
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!RPC_DATA->get_addr_by_hostid(recipient, svr_addr)) {
        ADAFS_DATA->spdlogger()->error("server address not resolvable for host id {}", recipient);
        return 1;
    }
    auto ret = HG_Create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_srv_remove_dentry_id(), &handle);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("creating handle FAILED");
        return 1;
    }
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(RPC_DATA->client_mid(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = HG_Get_output(handle, &out);

        ADAFS_DATA->spdlogger()->debug("Got response deleted inode: {}", out.del_inode);
        del_inode = static_cast<fuse_ino_t>(out.del_inode);

        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        ADAFS_DATA->spdlogger()->error("RPC send_remove_dentry (timed out)");
    }

    in.filename = nullptr; // XXX temporary. If this is not done free input crashes because of invalid pointer?!
    HG_Free_input(handle, &in);
    HG_Destroy(handle);
    if (del_inode == INVALID_INODE)
        err = 1;

    return err;
}


