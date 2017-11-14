
#include <preload/rpc/ld_rpc_data.hpp>


using namespace std;

int rpc_send_write(const hg_id_t ipc_write_data_id, const hg_id_t rpc_write_data_id, const string& path,
                   const size_t in_size, const off_t in_offset, const void* buf, size_t& write_size,
                   const bool append, const off_t updated_size) {

    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_write_data_in_t in{};
    rpc_data_out_t out{};
    int err;
    hg_return_t ret;
    margo_instance_id used_mid;
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
        ret = margo_create(ld_margo_ipc_id(), daemon_addr(), ipc_write_data_id, &handle);
        ld_logger->debug("{}() send to local daemon (IPC)", __func__);
        used_mid = ld_margo_ipc_id();
    } else { // remote
        // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
        if (!get_addr_by_hostid(recipient, svr_addr)) {
            ld_logger->error("{}() server address not resolvable for host id {}", __func__, recipient);
            return 1;
        }
        ret = margo_create(ld_margo_rpc_id(), svr_addr, rpc_write_data_id, &handle);
        ld_logger->debug("{}() send to remote daemon (RPC)", __func__);
        used_mid = ld_margo_ipc_id();
    }
    if (ret != HG_SUCCESS) {
        ld_logger->error("{}() creating handle FAILED", __func__);
        return 1;
    }
    /* register local target buffer for bulk access */
    // remove constness from buffer for transfer
    void* b_buf = const_cast<void*>(buf);
    ret = margo_bulk_create(used_mid, 1, &b_buf, &in_size, HG_BULK_READ_ONLY, &in.bulk_handle);
    if (ret != 0)
        ld_logger->error("{}() failed to create bulk on client", __func__);

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
        err = out.res;
        write_size = static_cast<size_t>(out.io_size);
        ld_logger->debug("{}() Got response {}", __func__, out.res);
        /* clean up resources consumed by this rpc */
        margo_bulk_free(in.bulk_handle);
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out", __func__);
        err = EAGAIN;
    }

    margo_destroy(handle);

    return err;
}