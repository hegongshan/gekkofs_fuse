//
// Created by aelycia on 9/13/17.
//

#include <preload/margo_ipc.hpp>
#include <cassert>

using namespace std;

static int max_retries = 3;

void send_minimal_rpc(const hg_id_t minimal_id) {

    hg_handle_t handle;
    rpc_minimal_in_tt in;
    rpc_minimal_out_tt out;

    printf("minimal RPC is running...\n");

    /* create handle */
    auto ret = HG_Create(ld_mercury_context(), daemon_addr(), minimal_id, &handle);
    assert(ret == HG_SUCCESS);

    /* Send rpc. Note that we are also transmitting the bulk handle in the
     * input struct.  It was set above.
     */
    in.input = 42;
    printf("About to send RPC\n");
    margo_forward(ld_margo_id(), handle, &in);
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

int ipc_send_open(const char* path, int flags, const mode_t mode, const hg_id_t ipc_open_id) {
    hg_handle_t handle;
    ipc_open_in_t in;
    ipc_res_out_t out;
    // fill in
    in.mode = mode;
    in.flags = flags;
    in.path = path;
    hg_bool_t success = HG_FALSE;
    auto ret = HG_Create(ld_mercury_context(), daemon_addr(), ipc_open_id, &handle);
    if (ret != HG_SUCCESS) {
        DAEMON_DEBUG0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    DAEMON_DEBUG0(debug_fd, "About to send open IPC to daemon\n");
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(ld_margo_id(), handle, &in, RPC_TIMEOUT);
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

int ipc_send_unlink(const char* path, const hg_id_t ipc_unlink_id) {
    hg_handle_t handle;
    ipc_unlink_in_t in;
    ipc_res_out_t out;
    // fill in
    in.path = path;
    hg_bool_t success = HG_FALSE;
    auto ret = HG_Create(ld_mercury_context(), daemon_addr(), ipc_unlink_id, &handle);
    if (ret != HG_SUCCESS) {
        DAEMON_DEBUG0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    DAEMON_DEBUG0(debug_fd, "About to send unlink IPC to daemon\n");
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(ld_margo_id(), handle, &in, RPC_TIMEOUT);
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