
#include <preload/rpc/ld_rpc_data_ws.hpp>


using namespace std;

/**
 * Called by an argobots thread in pwrite() and sends all chunks that go to the same destination at once
 * @param _arg <struct write_args*>
 */
void rpc_send_write_abt(void* _arg) {
    auto* arg = static_cast<struct write_args*>(_arg);

    auto recipient_size = arg->chnk_ids->size();
    auto chnk_ids = *arg->chnk_ids;
    vector<char*> chnks(recipient_size);
    vector<size_t> buf_sizes(recipient_size * 2);
    auto buf_size = 0; // counter for how much of the buffer is already mapped into chunks
    auto chunk_offset = static_cast<size_t>(0);
    // if the first chunk is not the very first chunk in the buffer, the previous chunksizes have to be set as an offset
    if (chnk_ids[0] != arg->chnk_start)
        chunk_offset = ((chnk_ids[0] - arg->chnk_start) * CHUNKSIZE) - arg->in_offset;
    for (size_t i = 0; i < buf_sizes.size(); i++) {
        // even numbers contain the sizes of ids, while uneven contain the chunksize
        if (i < buf_sizes.size() / 2)
            buf_sizes[i] = sizeof(rpc_chnk_id_t);
        else {
            if (i == buf_sizes.size() / 2) { // first chunk which might have an offset
                if (arg->in_size < CHUNKSIZE)
                    buf_sizes[i] = static_cast<size_t>(arg->in_size);
                else if (chunk_offset == 0) // if the first chunk is the very first chunk in the buffer
                    buf_sizes[i] = static_cast<size_t>(CHUNKSIZE - arg->in_offset);
                else
                    buf_sizes[i] = CHUNKSIZE;
            } else if (i + 1 == buf_sizes.size()) {// last chunk has remaining size
                buf_sizes[i] = arg->in_size - buf_size;
            } else {
                buf_sizes[i] = CHUNKSIZE;
            }

            // position the pointer according to the chunk number this code is executed for the second chunk+
            chnks[i - chnks.size()] = static_cast<char*>(const_cast<void*>(arg->buf)) + chunk_offset + buf_size;
            buf_size += buf_sizes[i];
        }
    }
    // setting pointers to the ids and to the chunks
    vector<void*> buf_ptrs(recipient_size * 2);
    for (unsigned long i = 0; i < buf_ptrs.size(); i++) {
        if (i < buf_sizes.size() / 2) // id pointer
            buf_ptrs[i] = &chnk_ids[i];
        else // data pointer
            buf_ptrs[i] = chnks[i - chnk_ids.size()];
    }
    // RPC
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_write_data_in_t in{};
    rpc_data_out_t out{};
    int err;
    hg_return_t ret;
    auto write_size = static_cast<size_t>(0);
    // fill in
    arg->path->c_str();
    in.path = arg->path->c_str();
    in.offset = (chunk_offset == 0) ? arg->in_offset : 0;

    margo_create_wrap(ipc_write_data_id, rpc_write_data_id, arg->recipient, handle, svr_addr, false);

    auto used_mid = margo_hg_handle_get_instance(handle);

    /* register local target buffer for bulk access */
    ret = margo_bulk_create(used_mid, static_cast<uint32_t>(buf_sizes.size()), buf_ptrs.data(), buf_sizes.data(),
                            HG_BULK_READ_ONLY, &in.bulk_handle);

    if (ret != HG_SUCCESS) {
        ld_logger->error("{}() failed to create bulk on client", __func__);
        ABT_eventual_set(*(arg->eventual), &write_size, sizeof(write_size));
        return;
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
        ret = margo_get_output(handle, &out);
        if (ret != HG_SUCCESS) {
            ld_logger->error("{}() failed to get rpc output", __func__);
            ABT_eventual_set(*(arg->eventual), &write_size, sizeof(write_size));
            return;
        }
        err = out.res;
        if (err != 0)
            write_size = static_cast<size_t>(0);
        else
            write_size = static_cast<size_t>(out.io_size);
        ld_logger->debug("{}() Got response {}", __func__, out.res);
        /* clean up resources consumed by this rpc */
        margo_bulk_free(in.bulk_handle);
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out", __func__);
        ABT_eventual_set(*(arg->eventual), &write_size, sizeof(write_size));
        return;
    }
    // Signal calling process that RPC is finished and put written size into return value
    ABT_eventual_set(*(arg->eventual), &write_size, sizeof(write_size));
    margo_destroy(handle);

}

void rpc_send_read_abt(void* _arg) {

    // Prepare buffers
    auto* arg = static_cast<struct read_args*>(_arg);
    auto recipient_size = arg->chnk_ids->size();
    auto chnk_ids = *arg->chnk_ids;
    vector<char*> chnks(recipient_size);
    vector<size_t> buf_sizes(recipient_size * 2);
    auto buf_size = 0; // counter for how much of the buffer is already mapped into chunks
    auto chunk_offset = static_cast<size_t>(0);
    // if the first chunk is not the very first chunk in the buffer, the previous chunksizes have to be set as an offset
    if (chnk_ids[0] != arg->chnk_start)
        chunk_offset = ((chnk_ids[0] - arg->chnk_start) * CHUNKSIZE) - arg->in_offset;
    for (size_t i = 0; i < buf_sizes.size(); i++) {
        // even numbers contain the sizes of ids, while uneven contain the chunksize
        if (i < buf_sizes.size() / 2)
            buf_sizes[i] = sizeof(rpc_chnk_id_t);
        else {
            if (i == buf_sizes.size() / 2) { // first chunk which might have an offset
                if (arg->in_size + arg->in_offset < CHUNKSIZE)
                    buf_sizes[i] = static_cast<size_t>(arg->in_size);
                else if (chunk_offset == 0) // if the first chunk is the very first chunk in the buffer
                    buf_sizes[i] = static_cast<size_t>(CHUNKSIZE - arg->in_offset);
                else
                    buf_sizes[i] = CHUNKSIZE;
            } else if (i + 1 == buf_sizes.size()) {// last chunk has remaining size
                buf_sizes[i] = arg->in_size - buf_size;
            } else {
                buf_sizes[i] = CHUNKSIZE;
            }

            // position the pointer according to the chunk number this code is executed for the second chunk+
            chnks[i - chnks.size()] = static_cast<char*>(const_cast<void*>(arg->buf)) + chunk_offset + buf_size;
            buf_size += buf_sizes[i];
        }
    }
    // setting pointers to the ids and to the chunks
    vector<void*> buf_ptrs(recipient_size * 2);
    for (unsigned long i = 0; i < buf_ptrs.size(); i++) {
        if (i < buf_sizes.size() / 2) // id pointer
            buf_ptrs[i] = &chnk_ids[i];
        else // data pointer
            buf_ptrs[i] = chnks[i - chnk_ids.size()];
    }

    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_read_data_in_t in{};
    rpc_data_out_t out{};
//    int err; // XXX
    hg_return_t ret;
    auto read_size = static_cast<size_t>(0);
    // fill in
    in.path = arg->path->c_str();
    in.size = arg->in_size;
    in.offset = (chunk_offset == 0) ? arg->in_offset : 0;

    margo_create_wrap(ipc_read_data_id, rpc_read_data_id, arg->recipient, handle, svr_addr, false);

    auto used_mid = margo_hg_handle_get_instance(handle);
    /* register local target buffer for bulk access */
    ret = margo_bulk_create(used_mid, static_cast<uint32_t>(buf_sizes.size()), buf_ptrs.data(), buf_sizes.data(),
                            HG_BULK_READWRITE, &in.bulk_handle);
    if (ret != HG_SUCCESS) {
        ld_logger->error("{}() failed to create bulk on client", __func__);
        ABT_eventual_set(*(arg->eventual), &read_size, sizeof(read_size));
        return;
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
        ret = margo_get_output(handle, &out);
        if (ret != HG_SUCCESS) {
            ld_logger->error("{}() failed to get rpc output", __func__);
            ABT_eventual_set(*(arg->eventual), &read_size, sizeof(read_size));
            return;
        }
        read_size = static_cast<size_t>(out.io_size);
//        err = out.res;
        ld_logger->debug("{}() Got response {}", __func__, out.res);
        /* clean up resources consumed by this rpc */
        margo_bulk_free(in.bulk_handle);
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out", __func__);
        ABT_eventual_set(*(arg->eventual), &read_size, sizeof(read_size));
        return;
//        err = EAGAIN;
    }
    // Signal calling process that RPC is finished and put read size into return value
    ABT_eventual_set(*(arg->eventual), &read_size, sizeof(read_size));

    margo_destroy(handle);
}

/**
 * XXX Currently unused.
 * @param path
 * @param in_size
 * @param in_offset
 * @param buf
 * @param write_size
 * @param append
 * @param updated_size
 * @return
 */
int rpc_send_write(const string& path, const size_t in_size, const off64_t in_offset, void* buf, size_t& write_size,
                   const bool append, const off64_t updated_size) {

    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_write_data_in_t in{};
    rpc_data_out_t out{};
    int err;
    hg_return_t ret;
    // fill in
    in.path = path.c_str();
    in.offset = in_offset;

    margo_create_wrap(ipc_write_data_id, rpc_write_data_id, path, handle, svr_addr, false);

    auto used_mid = margo_hg_handle_get_instance(handle);

    /* register local target buffer for bulk access */
    ret = margo_bulk_create(used_mid, 1, &buf, &in_size, HG_BULK_READ_ONLY, &in.bulk_handle);
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