//
// Created by aelycia on 9/13/17.
//

#include <preload/margo_ipc.hpp>
#include <boost/token_functions.hpp>
#include <boost/tokenizer.hpp>

void send_minimal_ipc(const hg_id_t minimal_id) {

    hg_handle_t handle;
    rpc_minimal_in_t in{};
    rpc_minimal_out_t out{};

    printf("minimal RPC is running...\n");

    /* create handle */
    auto ret = margo_create(ld_margo_ipc_id(), daemon_addr(), minimal_id, &handle);
    assert(ret == HG_SUCCESS);

    /* Send rpc. Note that we are also transmitting the bulk handle in the
     * input struct.  It was set above.
     */
    in.input = 42;
    printf("About to send RPC\n");
    margo_forward(handle, &in);
    printf("Waiting for response\n");
    /* decode response */
    ret = margo_get_output(handle, &out);
    assert(ret == HG_SUCCESS);

    printf("Got response ret: %d\n", out.output);

    /* clean up resources consumed by this rpc */
    margo_free_output(handle, &out);
    margo_destroy(handle);

    printf("minimal RPC is done.\n");
}

bool ipc_send_get_fs_config(const hg_id_t ipc_get_config_id) {
    hg_handle_t handle;
    ipc_config_in_t in{};
    ipc_config_out_t out{};
    // fill in
    in.dummy = 0; // XXX should be removed. havent checked yet how empty input with margo works
    auto ret = margo_create(ld_margo_ipc_id(), daemon_addr(), ipc_get_config_id, &handle);
    if (ret != HG_SUCCESS) {
        LD_LOG_DEBUG0(debug_fd, "creating handle FAILED\n");
        return false;
    }
    LD_LOG_DEBUG0(debug_fd, "About to send get config IPC to daemon\n");
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
        if (ret == HG_SUCCESS) {
            fs_config->mountdir = out.mountdir;
            fs_config->rootdir = out.rootdir;
            fs_config->atime_state = out.atime_state;
            fs_config->mtime_state = out.mtime_state;
            fs_config->ctime_state = out.ctime_state;
            fs_config->uid_state = out.uid_state;
            fs_config->gid_state = out.gid_state;
            fs_config->inode_no_state = out.inode_no_state;
            fs_config->link_cnt_state = out.link_cnt_state;
            fs_config->blocks_state = out.blocks_state;
            fs_config->uid = out.uid;
            fs_config->gid = out.gid;
            fs_config->host_id = out.host_id;
            fs_config->host_size = out.host_size;
            fs_config->rpc_port = to_string(RPCPORT);

            // split comma separated host string and create a hosts map
            string hosts_raw = out.hosts_raw;
            std::map<uint64_t, std::string> hostmap;
            boost::char_separator<char> sep(",");
            boost::tokenizer<boost::char_separator<char>> tok(hosts_raw, sep);
            uint64_t i = 0;
            for (auto&& s : tok) {
                hostmap[i++] = s;
            }
            fs_config->hosts = hostmap;
            LD_LOG_DEBUG(debug_fd, "Got response with mountdir: %s\n", out.mountdir);
        } else {
            printf("[ERR] Retrieving fs configurations from daemon");
        }
        /* clean up resources consumed by this rpc */
        out.rootdir = nullptr;
        out.mountdir = nullptr;
        out.hosts_raw = nullptr;
        margo_free_output(handle, &out);
    } else {
        LD_LOG_ERROR0(debug_fd, "IPC send_get_config (timed out)\n");
    }

    margo_free_input(handle, &in);
    margo_destroy(handle);
    return ret == HG_SUCCESS;
}

int ipc_send_open(const string& path, int flags, const mode_t mode, const hg_id_t ipc_open_id) {
    hg_handle_t handle;
    ipc_open_in_t in{};
    ipc_err_out_t out{};
    // fill in
    in.mode = mode;
    in.flags = flags;
    in.path = path.c_str();
    int err = EUNKNOWN;
    auto ret = margo_create(ld_margo_ipc_id(), daemon_addr(), ipc_open_id, &handle);
    if (ret != HG_SUCCESS) {
        LD_LOG_DEBUG0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    LD_LOG_DEBUG0(debug_fd, "About to send open IPC to daemon\n");
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
        LD_LOG_ERROR0(debug_fd, "IPC send_open (timed out)\n");
    }

    in.path = nullptr; // XXX temporary. If this is not done free input crashes because of invalid pointer?!

    margo_free_input(handle, &in);
    margo_destroy(handle);
    return err;
}

int ipc_send_stat(const string& path, string& attr, const hg_id_t ipc_stat_id) {
    hg_handle_t handle;
    ipc_stat_in_t in{};
    ipc_stat_out_t out{};
    // fill in
    in.path = path.c_str();
    int err;
    auto ret = margo_create(ld_margo_ipc_id(), daemon_addr(), ipc_stat_id, &handle);
    if (ret != HG_SUCCESS) {
        LD_LOG_DEBUG0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    LD_LOG_DEBUG0(debug_fd, "About to send stat IPC to daemon\n");
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
        if (out.err == 0) {
            attr = out.db_val;
        }
        /* clean up resources consumed by this rpc */
        out.db_val = nullptr;
        margo_free_output(handle, &out);
    } else {
        LD_LOG_ERROR0(debug_fd, "IPC send_stat (timed out)\n");
        err = 1;
    }

    in.path = nullptr; // XXX temporary. If this is not done free input crashes because of invalid pointer?!

    margo_free_input(handle, &in);
    margo_destroy(handle);
    return err;
}

int ipc_send_unlink(const string& path, const hg_id_t ipc_unlink_id) {
    hg_handle_t handle;
    ipc_unlink_in_t in{};
    ipc_err_out_t out{};
    // fill in
    in.path = path.c_str();
    int err = EUNKNOWN;
    auto ret = margo_create(ld_margo_ipc_id(), daemon_addr(), ipc_unlink_id, &handle);
    if (ret != HG_SUCCESS) {
        LD_LOG_DEBUG0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    LD_LOG_DEBUG0(debug_fd, "About to send unlink IPC to daemon\n");
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
        LD_LOG_ERROR0(debug_fd, "IPC send_unlink (timed out)\n");
    }

    in.path = nullptr; // XXX temporary. If this is not done free input crashes because of invalid pointer?!

    margo_free_input(handle, &in);
    margo_destroy(handle);
    return err;
}