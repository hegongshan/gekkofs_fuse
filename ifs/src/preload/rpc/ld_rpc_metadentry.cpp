//
// Created by evie on 9/7/17.
//

#include <preload/rpc/ld_rpc_metadentry.hpp>
#include <rpc/rpc_utils.hpp>


using namespace std;

static int max_retries = 3;

void send_minimal_rpc(const hg_id_t minimal_id) {

//    hg_handle_t handle;
//    rpc_minimal_in_t in{};
//    rpc_minimal_out_t out{};
//    hg_addr_t svr_addr = HG_ADDR_NULL;
////    hg_addr_t* svr_addr = static_cast<hg_addr_t*>(arg);
//
//    ADAFS_DATA->spdlogger()->debug("Looking up address");
//
//    margo_addr_lookup(RPC_DATA->client_mid(), "bmi+tcp://134.93.182.11:1234"s.c_str(), &svr_addr);
//
//    ADAFS_DATA->spdlogger()->debug("minimal RPC is running...");
//
//
//    /* create handle */
//    auto ret = margo_create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_minimal_id(), &handle);
//    if (ret != HG_SUCCESS) {
//        printf("Creating handle FAILED\n");
//        return;
//    }
//
//    /* Send rpc. Note that we are also transmitting the bulk handle in the
//     * input struct.  It was set above.
//     */
//    in.input = 42;
//    ADAFS_DATA->spdlogger()->debug("About to call RPC ...");
//    int send_ret = HG_FALSE;
//    for (int i = 1; i < max_retries; ++i) {
//        send_ret = margo_forward_timed(RPC_DATA->client_mid(), handle, &in, 5);
//        if (send_ret == HG_SUCCESS) {
//            break;
//        }
//    }
//
//    if (send_ret == HG_SUCCESS) {
//        /* decode response */
//        ret = margo_get_output(handle, &out);
//
//        ADAFS_DATA->spdlogger()->debug("Got response ret: {}", out.output);
//
//        /* clean up resources consumed by this rpc */
//        margo_free_output(handle, &out);
//    } else {
//        ADAFS_DATA->spdlogger()->info("RPC NOT send (timed out)");
//    }
//    HG_Addr_free(margo_get_class(RPC_DATA->client_mid()), svr_addr);
//    margo_free_input(handle, &in);
//    margo_destroy(handle);
//
//    ADAFS_DATA->spdlogger()->debug("minimal RPC is done.");
}

int rpc_send_create_node(const hg_id_t rpc_create_node_id, const size_t recipient, const std::string& path,
                         const mode_t mode) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_create_node_in_t in{};
    rpc_err_out_t out{};
    // fill in
    in.path = path.c_str();
    in.mode = mode;
    int err = EUNKNOWN;
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!get_addr_by_hostid(recipient, svr_addr)) {
        LD_LOG_ERROR(debug_fd, "server address not resolvable for host id %lu\n", recipient);
        return 1;
    }
    auto ret = margo_create(ld_margo_rpc_id(), svr_addr, rpc_create_node_id, &handle);
    if (ret != HG_SUCCESS) {
        LD_LOG_ERROR0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    int send_ret = HG_FALSE;
    LD_LOG_TRACE0(debug_fd, "About to send create_node RPC ...\n");
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = margo_get_output(handle, &out);

        LD_LOG_TRACE(debug_fd, "Got response success: %d\n", out.err);
        err = out.err;
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        LD_LOG_ERROR0(debug_fd, "RPC send_create_node (timed out)\n");
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
        LD_LOG_ERROR(debug_fd, "server address not resolvable for host id %lu\n", recipient);
        return 1;
    }
    auto ret = margo_create(ld_margo_rpc_id(), svr_addr, rpc_get_attr_id, &handle);
    if (ret != HG_SUCCESS) {
        LD_LOG_ERROR0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = margo_get_output(handle, &out);
        LD_LOG_TRACE(debug_fd, "Got response success: %d\n", out.err);
        err = out.err;
        if (out.err == 0)
            attr = out.db_val;

        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        err = 1;
        LD_LOG_ERROR0(debug_fd, "RPC send_get_attr (timed out)\n");
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
        LD_LOG_ERROR(debug_fd, "server address not resolvable for host id %d\n", static_cast<int>(recipient));
        return 1;
    }
    auto ret = margo_create(ld_margo_rpc_id(), svr_addr, rpc_remove_node_id, &handle);
    if (ret != HG_SUCCESS) {
        LD_LOG_ERROR0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = margo_get_output(handle, &out);

        LD_LOG_DEBUG(debug_fd, "Got response success: %d\n", out.err);
        err = out.err;
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        LD_LOG_ERROR0(debug_fd, "RPC send_remove_node (timed out)\n");
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
        LD_LOG_TRACE0(debug_fd, "rpc_send_update_metadentry to local daemon (IPC)\n");
    } else { // remote
        // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
        if (!get_addr_by_hostid(recipient, svr_addr)) {
            LD_LOG_ERROR(debug_fd, "server address not resolvable for host id %lu\n", recipient);
            return 1;
        }
        ret = margo_create(ld_margo_rpc_id(), svr_addr, rpc_update_metadentry_id, &handle);
        LD_LOG_TRACE0(debug_fd, "rpc_send_update_metadentry to remote daemon (RPC)\n");
    }
    if (ret != HG_SUCCESS) {
        LD_LOG_ERROR0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    LD_LOG_DEBUG0(debug_fd, "About to send update metadentry RPC to daemon\n");
    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        LD_LOG_DEBUG0(debug_fd, "Waiting for response\n");
        ret = margo_get_output(handle, &out);

        LD_LOG_DEBUG(debug_fd, "Got response success: %d\n", out.err);
        err = out.err;
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        LD_LOG_ERROR0(debug_fd, "RPC send_update_metadentry (timed out)\n");
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
        LD_LOG_TRACE0(debug_fd, "rpc_send_update_metadentry_size to local daemon (IPC)\n");
    } else { // remote
        // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
        if (!get_addr_by_hostid(recipient, svr_addr)) {
            LD_LOG_ERROR(debug_fd, "server address not resolvable for host id %lu\n", recipient);
            return 1;
        }
        ret = margo_create(ld_margo_rpc_id(), svr_addr, rpc_update_metadentry_size_id, &handle);
        LD_LOG_TRACE0(debug_fd, "rpc_send_update_metadentry_size to remote daemon (RPC)\n");
    }
    if (ret != HG_SUCCESS) {
        LD_LOG_ERROR0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    LD_LOG_DEBUG0(debug_fd, "About to send update metadentry size RPC to daemon\n");
    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        LD_LOG_DEBUG0(debug_fd, "Waiting for response\n");
        ret = margo_get_output(handle, &out);

        LD_LOG_DEBUG(debug_fd, "Got response success: %d\n", out.err);
        err = out.err;
        ret_size = out.ret_size;
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        LD_LOG_ERROR0(debug_fd, "RPC send_update_metadentry_size (timed out)\n");
    }

    margo_destroy(handle);
    return err;
}