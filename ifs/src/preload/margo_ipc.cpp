#include <global/configure.hpp>
#include <preload/margo_ipc.hpp>
#include <boost/token_functions.hpp>
#include <boost/tokenizer.hpp>

void send_minimal_ipc(const hg_id_t minimal_id) {

    hg_handle_t handle;
    rpc_minimal_in_t in{};
    rpc_minimal_out_t out{};

    printf("minimal RPC is running...\n");

    /* create handle */
    auto ret = margo_create(ld_margo_ipc_id, daemon_svr_addr, minimal_id, &handle);
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

/**
 * Gets fs configuration information from the running daemon and transfers it to the memory of the library
 * @return
 */
bool ipc_send_get_fs_config() {
    hg_handle_t handle;
    ipc_config_in_t in{};
    ipc_config_out_t out{};
    // fill in
    in.dummy = 0; // XXX should be removed. havent checked yet how empty input with margo works
    auto ret = margo_create(ld_margo_ipc_id, daemon_svr_addr, ipc_config_id, &handle);
    if (ret != HG_SUCCESS) {
        CTX->log()->error("{}() creating handle for failed", __func__);
        return false;
    }
    CTX->log()->debug("{}() About to send get config IPC to daemon", __func__);
    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        CTX->log()->debug("{}() Waiting for response", __func__);
        ret = margo_get_output(handle, &out);
        if (ret == HG_SUCCESS) {
            if (!CTX->mountdir().empty() && CTX->mountdir() != out.mountdir) {
                CTX->log()->warn(
                        "{}() fs_config mountdir {} and received out.mountdir {} mismatch detected! Using received mountdir",
                        __func__, CTX->mountdir(), out.mountdir);
                CTX->mountdir(out.mountdir);
            }
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
            fs_config->rpc_port = to_string(RPC_PORT);

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
            CTX->log()->debug("{}() Got response with mountdir {}", __func__, out.mountdir);
        } else {
            CTX->log()->error("{}() Retrieving fs configurations from daemon", __func__);
        }
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        CTX->log()->warn("{}() timed out", __func__);
    }

    margo_destroy(handle);
    return ret == HG_SUCCESS;
}