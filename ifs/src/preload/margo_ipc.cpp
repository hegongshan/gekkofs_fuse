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
        ld_logger->error("{}() creating handle for failed", __func__);
        return false;
    }
    ld_logger->debug("{}() About to send get config IPC to daemon", __func__);
    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ld_logger->debug("{}() Waiting for response", __func__);
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
            ld_logger->debug("{}() Got response with mountdir {}", __func__, out.mountdir);
        } else {
            ld_logger->error("{}() Retrieving fs configurations from daemon", __func__);
        }
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out", __func__);
    }

    margo_destroy(handle);
    return ret == HG_SUCCESS;
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
        ld_logger->error("{}() creating handle failed", __func__);
        return 1;
    }
    ld_logger->debug("{}() About to send unlink IPC to daemon", __func__);
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