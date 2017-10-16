//
// Created by evie on 9/8/17.
//

#include <preload/rpc/ld_rpc_data.hpp>


using namespace std;

int rpc_send_write(const hg_id_t ipc_write_data_id, const hg_id_t rpc_write_data_id, const string& path,
                   const size_t in_size, const off_t in_offset, const void* buf, size_t& write_size,
                   const bool append, const off_t updated_size) {

    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    bool local_op = true;
    rpc_write_data_in_t in;
    rpc_data_out_t out;
    int err;
    hg_return_t ret;
    // fill in
    in.path = path.c_str();
    in.size = in_size;
    in.offset = in_offset;
    in.updated_size = updated_size;
    if (append)
        in.append = HG_TRUE;
    else
        in.append = HG_FALSE;

    auto recipient = get_rpc_node(path);
    if (is_local_op(recipient)) { // local
        ret = HG_Create(margo_get_context(ld_margo_ipc_id()), daemon_addr(), ipc_write_data_id, &handle);
        LD_LOG_TRACE0(debug_fd, "rpc_send_write to local daemon (IPC)\n");
    } else { // remote
        local_op = false;
        // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
        if (!get_addr_by_hostid(recipient, svr_addr)) {
            LD_LOG_ERROR(debug_fd, "server address not resolvable for host id %lu\n", recipient);
            return 1;
        }
        ret = HG_Create(margo_get_context(ld_margo_rpc_id()), svr_addr, rpc_write_data_id, &handle);
        LD_LOG_TRACE0(debug_fd, "rpc_send_write to remote daemon (RPC)\n");
    }
    if (ret != HG_SUCCESS) {
        LD_LOG_ERROR0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    auto hgi = HG_Get_info(handle);
    /* register local target buffer for bulk access */
    // remove constness from buffer for transfer
    void* b_buf = const_cast<void*>(buf);
    ret = HG_Bulk_create(hgi->hg_class, 1, &b_buf, &in_size, HG_BULK_READ_ONLY, &in.bulk_handle);
    if (ret != 0)
        LD_LOG_ERROR0(debug_fd, "failed to create bulk on client\n");

    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(local_op ? ld_margo_ipc_id() : ld_margo_rpc_id(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {

        /* decode response */
        ret = HG_Get_output(handle, &out);
        err = out.res;
        write_size = static_cast<size_t>(out.io_size);
        LD_LOG_TRACE(debug_fd, "Got response %d\n", out.res);
        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        LD_LOG_ERROR0(debug_fd, "RPC rpc_send_write (timed out)");
        err = EAGAIN;
    }

    in.path = nullptr;

    HG_Bulk_free(in.bulk_handle);
    HG_Free_input(handle, &in);
    HG_Destroy(handle);

    return err;
}