//
// Created by evie on 9/7/17.
//

#include <preload/rpc/ld_rpc_metadentry.hpp>
#include <rpc/rpc_utils.hpp>


using namespace std;

int rpc_send_open(const hg_id_t ipc_open_id, const hg_id_t rpc_open_id, const std::string& path, const mode_t mode,
                  const int flags) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_open_in_t in{};
    rpc_err_out_t out{};
    hg_return_t ret;
    int err = EUNKNOWN;
    // fill in
    in.path = path.c_str();
    in.mode = mode;

    // TODO handle all flags. currently only file create
    auto recipient = get_rpc_node(path);
    if (is_local_op(recipient)) { // local
        ret = margo_create(ld_margo_ipc_id(), daemon_addr(), ipc_open_id, &handle);
        ld_logger->debug("{}() to local daemon (IPC)", __func__);
    } else { // remote
        // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
        if (!get_addr_by_hostid(recipient, svr_addr)) {
            ld_logger->error("{}() server address not resolvable for host id {}", __func__, recipient);
            return 1;
        }
        ret = margo_create(ld_margo_rpc_id(), svr_addr, rpc_open_id, &handle);
        ld_logger->debug("{}() to remote daemon (RPC)", __func__);
    }
    if (ret != HG_SUCCESS) {
        ld_logger->error("{}() creating handle FAILED", __func__);
        return 1;
    }
    int send_ret = HG_FALSE;
    ld_logger->debug("{}() About to send RPC ...", __func__);
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = margo_get_output(handle, &out);

        ld_logger->debug("{}() Got response success: {}", __func__, out.err);
        err = out.err;
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out");
    }

    margo_destroy(handle);
    return err;
}

int rpc_send_get_attr(const hg_id_t rpc_get_attr_id, const size_t recipient, const std::string& path, string& attr) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_get_attr_in_t in{};
    rpc_get_attr_out_t out{};
    // fill in
    in.path = path.c_str();
    int err;
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!get_addr_by_hostid(recipient, svr_addr)) {
        ld_logger->error("{}() server address not resolvable for host id {}", __func__, recipient);
        return 1;
    }
    auto ret = margo_create(ld_margo_rpc_id(), svr_addr, rpc_get_attr_id, &handle);
    if (ret != HG_SUCCESS) {
        ld_logger->error("{}() creating handle FAILED", __func__);
        return 1;
    }
    ld_logger->debug("{}() About to send RPC ...", __func__);
    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = margo_get_output(handle, &out);
        ld_logger->debug("{}() Got response success: {}", __func__, out.err);
        err = out.err;
        if (out.err == 0)
            attr = out.db_val;

        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        err = 1;
        ld_logger->warn("{}() timed out", __func__);
    }
    margo_destroy(handle);
    return err;
}

int rpc_send_remove_node(const hg_id_t rpc_remove_node_id, const size_t recipient, const std::string& path) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_remove_node_in_t in{};
    rpc_err_out_t out{};
    // fill in
    in.path = path.c_str();
    int err = EUNKNOWN;
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!get_addr_by_hostid(recipient, svr_addr)) {
        ld_logger->error("{}() server address not resolvable for host id {}", __func__, static_cast<int>(recipient));
        return 1;
    }
    auto ret = margo_create(ld_margo_rpc_id(), svr_addr, rpc_remove_node_id, &handle);
    if (ret != HG_SUCCESS) {
        ld_logger->error("{}() creating handle FAILED", __func__);
        return 1;
    }
    ld_logger->debug("{}() About to send RPC ...", __func__);
    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = margo_get_output(handle, &out);

        ld_logger->debug("{}() Got response success: {}", __func__, out.err);
        err = out.err;
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out", __func__);
    }
    margo_destroy(handle);
    return err;
}


int rpc_send_update_metadentry(const hg_id_t ipc_update_metadentry_id, const hg_id_t rpc_update_metadentry_id,
                               const string& path, const Metadentry& md, const MetadentryUpdateFlags& md_flags) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_update_metadentry_in_t in{};
    rpc_err_out_t out{};
    int err = EUNKNOWN;
    hg_return_t ret;
    // fill in
    // add data
    in.path = path.c_str();
    in.size = md_flags.size ? md.size : 0;
    in.nlink = md_flags.link_count ? md.link_count : 0;
    in.gid = md_flags.gid ? md.gid : 0;
    in.uid = md_flags.uid ? md.uid : 0;
    in.blocks = md_flags.blocks ? md.blocks : 0;
    in.inode_no = md_flags.inode_no ? md.inode_no : 0;
    in.atime = md_flags.atime ? md.atime : 0;
    in.mtime = md_flags.mtime ? md.mtime : 0;
    in.ctime = md_flags.ctime ? md.ctime : 0;
    // add data flags
    in.size_flag = bool_to_merc_bool(md_flags.size);
    in.nlink_flag = bool_to_merc_bool(md_flags.link_count);
    in.gid_flag = bool_to_merc_bool(md_flags.gid);
    in.uid_flag = bool_to_merc_bool(md_flags.uid);
    in.block_flag = bool_to_merc_bool(md_flags.blocks);
    in.inode_no_flag = bool_to_merc_bool(md_flags.inode_no);
    in.atime_flag = bool_to_merc_bool(md_flags.atime);
    in.mtime_flag = bool_to_merc_bool(md_flags.mtime);
    in.ctime_flag = bool_to_merc_bool(md_flags.ctime);

    auto recipient = get_rpc_node(path);
    if (is_local_op(recipient)) { // local
        ret = margo_create(ld_margo_ipc_id(), daemon_addr(), ipc_update_metadentry_id, &handle);
        ld_logger->debug("{}() to local daemon (IPC)");
    } else { // remote
        // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
        if (!get_addr_by_hostid(recipient, svr_addr)) {
            ld_logger->error("{}() server address not resolvable for host id {}", __func__, recipient);
            return 1;
        }
        ret = margo_create(ld_margo_rpc_id(), svr_addr, rpc_update_metadentry_id, &handle);
        ld_logger->debug("{}() to remote daemon (RPC)", __func__);
    }
    if (ret != HG_SUCCESS) {
        ld_logger->error("{}() creating handle FAILED", __func__);
        return 1;
    }
    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ld_logger->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(handle, &out);

        ld_logger->debug("{}() Got response success: {}", __func__, out.err);
        err = out.err;
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out", __func__);
    }

    margo_destroy(handle);
    return err;
}

int rpc_send_update_metadentry_size(const hg_id_t ipc_update_metadentry_size_id,
                                    const hg_id_t rpc_update_metadentry_size_id, const string& path, const off_t size,
                                    const bool append_flag, off_t& ret_size) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_update_metadentry_size_in_t in{};
    rpc_update_metadentry_size_out_t out{};
    // add data
    in.path = path.c_str();
    in.size = size;
    if (append_flag)
        in.append = HG_TRUE;
    else
        in.append = HG_FALSE;
    int err = EUNKNOWN;
    hg_return_t ret;
    auto recipient = get_rpc_node(path);
    if (is_local_op(recipient)) { // local
        ret = margo_create(ld_margo_ipc_id(), daemon_addr(), ipc_update_metadentry_size_id,
                           &handle);
        ld_logger->debug("{}() to local daemon (IPC)", __func__);
    } else { // remote
        // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
        if (!get_addr_by_hostid(recipient, svr_addr)) {
            ld_logger->error("{}() server address not resolvable for host id {}", __func__, recipient);
            return 1;
        }
        ret = margo_create(ld_margo_rpc_id(), svr_addr, rpc_update_metadentry_size_id, &handle);
        ld_logger->debug("{}() to remote daemon (RPC)", __func__);
    }
    if (ret != HG_SUCCESS) {
        ld_logger->error("{}() creating handle FAILED", __func__);
        return 1;
    }
    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ld_logger->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(handle, &out);

        ld_logger->debug("{}() Got response success: {}", __func__, out.err);
        err = out.err;
        ret_size = out.ret_size;
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out", __func__);
    }

    margo_destroy(handle);
    return err;
}