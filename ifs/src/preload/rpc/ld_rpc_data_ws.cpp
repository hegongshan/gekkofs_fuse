
#include <preload/rpc/ld_rpc_data_ws.hpp>


using namespace std;

/**
 * Called by an argobots thread in pwrite() and sends all chunks that go to the same destination at once
 * @param _arg <struct write_args*>
 */
void rpc_send_write_abt(void* _arg) {
    // Unpack
    auto* arg = static_cast<struct write_args*>(_arg);
    auto chnk_ids = *arg->chnk_ids;
    // RPC
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_write_data_in_t in{};
    rpc_data_out_t out{};
    hg_return_t ret;
    auto write_size = static_cast<size_t>(0);
    // fill in
    in.path = arg->path->c_str();
    in.offset = arg->in_offset;
    in.chunk_n = chnk_ids.size();
    in.chunk_start = arg->chnk_start;
    in.chunk_end = arg->chnk_end;
    in.total_chunk_size = arg->total_chunk_size;

    margo_create_wrap(ipc_write_data_id, rpc_write_data_id, arg->recipient, handle, svr_addr, false);

    auto used_mid = margo_hg_handle_get_instance(handle);

    // register local target buffer for bulk access
    auto bulk_buf = const_cast<void*>(arg->buf);
    ret = margo_bulk_create(used_mid, 1, &bulk_buf, &arg->in_size, HG_BULK_READ_ONLY, &in.bulk_handle);

    if (ret != HG_SUCCESS) {
        ld_logger->error("{}() failed to create bulk on client", __func__);
        ABT_eventual_set(arg->eventual, &write_size, sizeof(write_size));
        return;
    }

    for (int i = 0; i < RPC_TRIES; ++i) {
        // Wait for the RPC response.
        // This will call eventual_wait internally causing the calling ULT to be BLOCKED and implicitly yields
        ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (ret == HG_SUCCESS) {
            break;
        }
    }
    if (ret == HG_SUCCESS) {
        /* decode response */
        ret = margo_get_output(handle, &out);
        if (ret != HG_SUCCESS) {
            ld_logger->error("{}() failed to get rpc output", __func__);
            ABT_eventual_set(arg->eventual, &write_size, sizeof(write_size));
            return;
        }
        ld_logger->debug("{}() Got response {}", __func__, out.res);
        if (out.res != 0)
            errno = out.res;
        write_size = static_cast<size_t>(out.io_size);
        // Signal calling process that RPC is finished and put written size into return value
        ABT_eventual_set(arg->eventual, &write_size, sizeof(write_size));
        // clean up resources consumed by this rpc
        margo_bulk_free(in.bulk_handle);
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out", __func__);
        ABT_eventual_set(arg->eventual, &write_size, sizeof(write_size));
        return;
    }
    margo_destroy(handle);
}

void rpc_send_read_abt(void* _arg) {
    // Unpack
    auto* arg = static_cast<struct read_args*>(_arg);
    auto chnk_ids = *arg->chnk_ids;
    // RPC
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_read_data_in_t in{};
    rpc_data_out_t out{};
    hg_return_t ret;
    auto read_size = static_cast<size_t>(0);
    // fill in
    in.path = arg->path->c_str();
    in.offset = arg->in_offset;
    in.chunk_n = chnk_ids.size();
    in.chunk_start = arg->chnk_start;
    in.chunk_end = arg->chnk_end;
    in.total_chunk_size = arg->total_chunk_size;

    margo_create_wrap(ipc_read_data_id, rpc_read_data_id, arg->recipient, handle, svr_addr, false);

    auto used_mid = margo_hg_handle_get_instance(handle);

    // register local target buffer for bulk access
    ret = margo_bulk_create(used_mid, 1, &arg->buf, &arg->in_size, HG_BULK_WRITE_ONLY, &in.bulk_handle);

    if (ret != HG_SUCCESS) {
        ld_logger->error("{}() failed to create bulk on client", __func__);
        ABT_eventual_set(arg->eventual, &read_size, sizeof(read_size));
        return;
    }
    // Send RPC and wait for response
    for (int i = 0; i < RPC_TRIES; ++i) {
        // Wait for the RPC response.
        // This will call eventual_wait internally causing the calling ULT to be BLOCKED and implicitly yields
        ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (ret == HG_SUCCESS) {
            break;
        }
    }
    if (ret == HG_SUCCESS) {
        /* decode response */
        ret = margo_get_output(handle, &out);
        if (ret != HG_SUCCESS) {
            ld_logger->error("{}() failed to get rpc output", __func__);
            ABT_eventual_set(arg->eventual, &read_size, sizeof(read_size));
            return;
        }
        ld_logger->debug("{}() Got response {}", __func__, out.res);
        if (out.res != 0)
            errno = out.res;
        read_size = static_cast<size_t>(out.io_size);
        // Signal calling process that RPC is finished and put read size into return value
        ABT_eventual_set(arg->eventual, &read_size, sizeof(read_size));
        // clean up resources consumed by this rpc
        margo_bulk_free(in.bulk_handle);
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out", __func__);
        ABT_eventual_set(arg->eventual, &read_size, sizeof(read_size));
        return;
    }
    margo_destroy(handle);
}