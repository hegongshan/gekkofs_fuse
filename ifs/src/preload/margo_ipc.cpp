//
// Created by aelycia on 9/13/17.
//

#include <preload/margo_ipc.hpp>
#include <rpc/rpc_types.hpp>
#include <cassert>

using namespace std;

static int max_retries = 3;

void send_minimal_ipc(const hg_id_t minimal_id) {

    hg_handle_t handle;
    rpc_minimal_in_t in;
    rpc_minimal_out_t out;

    printf("minimal RPC is running...\n");

    /* create handle */
    auto ret = HG_Create(ld_mercury_ipc_context(), daemon_addr(), minimal_id, &handle);
    assert(ret == HG_SUCCESS);

    /* Send rpc. Note that we are also transmitting the bulk handle in the
     * input struct.  It was set above.
     */
    in.input = 42;
    printf("About to send RPC\n");
    margo_forward(ld_margo_ipc_id(), handle, &in);
    printf("Waiting for response\n");
    /* decode response */
    ret = HG_Get_output(handle, &out);
    assert(ret == HG_SUCCESS);

    printf("Got response ret: %d\n", out.output);

    /* clean up resources consumed by this rpc */
    HG_Free_output(handle, &out);
    HG_Destroy(handle);

    printf("minimal RPC is done.\n");
}

bool ipc_send_get_fs_config(const hg_id_t ipc_get_config_id) {
    hg_handle_t handle;
    ipc_config_in_t in;
    ipc_config_out_t out;
    // fill in
    in.dummy = 0; // XXX should be removed. havent checked yet how empty input with margo works
    auto ret = HG_Create(ld_mercury_ipc_context(), daemon_addr(), ipc_get_config_id, &handle);
    if (ret != HG_SUCCESS) {
        DAEMON_DEBUG0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    DAEMON_DEBUG0(debug_fd, "About to send get config IPC to daemon\n");
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(ld_margo_ipc_id(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        DAEMON_DEBUG0(debug_fd, "Waiting for response\n");
        ret = HG_Get_output(handle, &out);
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
            DAEMON_DEBUG(debug_fd, "Got response with mountdir: %s\n", out.mountdir);
        } else {
            printf("[ERR] Retrieving fs configurations from daemon");
        }
        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        DAEMON_DEBUG0(debug_fd, "IPC send_get_config (timed out)\n");
    }

    HG_Free_input(handle, &in);
    HG_Destroy(handle);
    return ret == HG_SUCCESS;
}

int ipc_send_open(const char* path, int flags, const mode_t mode, const hg_id_t ipc_open_id) {
    hg_handle_t handle;
    ipc_open_in_t in;
    ipc_res_out_t out;
    // fill in
    in.mode = mode;
    in.flags = flags;
    in.path = path;
    hg_bool_t success = HG_FALSE;
    auto ret = HG_Create(ld_mercury_ipc_context(), daemon_addr(), ipc_open_id, &handle);
    if (ret != HG_SUCCESS) {
        DAEMON_DEBUG0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    DAEMON_DEBUG0(debug_fd, "About to send open IPC to daemon\n");
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(ld_margo_ipc_id(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        DAEMON_DEBUG0(debug_fd, "Waiting for response\n");
        ret = HG_Get_output(handle, &out);

        DAEMON_DEBUG(debug_fd, "Got response success: %d\n", static_cast<bool>(out.res));
        success = out.res;
        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        DAEMON_DEBUG0(debug_fd, "IPC send_open (timed out)\n");
    }

    in.path = nullptr; // XXX temporary. If this is not done free input crashes because of invalid pointer?!

    HG_Free_input(handle, &in);
    HG_Destroy(handle);
    return success == HG_TRUE ? 0 : 1;
}

int ipc_send_stat(const char* path, struct stat* attr, const hg_id_t ipc_stat_id) {
    hg_handle_t handle;
    ipc_stat_in_t in;
    ipc_stat_out_t out;
    // fill in
    in.path = path;
    hg_bool_t success = HG_FALSE;
    auto ret = HG_Create(ld_mercury_ipc_context(), daemon_addr(), ipc_stat_id, &handle);
    if (ret != HG_SUCCESS) {
        DAEMON_DEBUG0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    DAEMON_DEBUG0(debug_fd, "About to send stat IPC to daemon\n");
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(ld_margo_ipc_id(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        DAEMON_DEBUG0(debug_fd, "Waiting for response\n");
        ret = HG_Get_output(handle, &out);

        DAEMON_DEBUG(debug_fd, "Got response success: %d\n", static_cast<bool>(out.res));
        success = out.res;
        if (out.res == HG_TRUE)
            db_val_to_stat(path, out.db_val, *attr);
        /* clean up resources consumed by this rpc */
        out.db_val = nullptr;
        HG_Free_output(handle, &out);
    } else {
        DAEMON_DEBUG0(debug_fd, "IPC send_stat (timed out)\n");
    }

    in.path = nullptr; // XXX temporary. If this is not done free input crashes because of invalid pointer?!

    HG_Free_input(handle, &in);
    HG_Destroy(handle);
    return success == HG_TRUE ? 0 : 1;
}

int ipc_send_unlink(const char* path, const hg_id_t ipc_unlink_id) {
    hg_handle_t handle;
    ipc_unlink_in_t in;
    ipc_res_out_t out;
    // fill in
    in.path = path;
    hg_bool_t success = HG_FALSE;
    auto ret = HG_Create(ld_mercury_ipc_context(), daemon_addr(), ipc_unlink_id, &handle);
    if (ret != HG_SUCCESS) {
        DAEMON_DEBUG0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    DAEMON_DEBUG0(debug_fd, "About to send unlink IPC to daemon\n");
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(ld_margo_ipc_id(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        DAEMON_DEBUG0(debug_fd, "Waiting for response\n");
        ret = HG_Get_output(handle, &out);

        DAEMON_DEBUG(debug_fd, "Got response success: %d\n", static_cast<bool>(out.res));
        success = out.res;
        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        DAEMON_DEBUG0(debug_fd, "IPC send_unlink (timed out)\n");
    }

    in.path = nullptr; // XXX temporary. If this is not done free input crashes because of invalid pointer?!

    HG_Free_input(handle, &in);
    HG_Destroy(handle);
    return success == HG_TRUE ? 0 : 1;
}