
#include <preload/rpc/ld_rpc_data.hpp>


using namespace std;

/**
 * Called by an argobots thread in pwrite() and sends all chunks that go to the same destination at once
 * @param _arg <struct write_args*>
 */
void rpc_send_write_abt(void* _arg) {
    auto* arg = static_cast<struct write_args*>(_arg);

    auto recipient_size = arg->chnk_ids.size();
    auto chnk_ids = arg->chnk_ids;
    vector<char*> chnks(recipient_size);
    vector<size_t> buf_sizes(recipient_size * 2);
    for (size_t i = 0; i < buf_sizes.size(); i++) {
        // even numbers contain the sizes of ids, while uneven contain the chunksize
        if (i % 2 == 0)
            buf_sizes[i] = sizeof(rpc_chnk_id_t);
        else {
            if (chnk_ids[i / 2] == chnk_ids.size() - 1) {// if current chunk size is last chunk
                // the last chunk will have the rest of the size, i.e., write size - all applied chunk sizes
                buf_sizes[i] = arg->in_size - (chnk_ids[i / 2] * CHUNKSIZE);
            } else {
                buf_sizes[i] = CHUNKSIZE;
            }
            // position the pointer according to the chunk number
            chnks[i / 2] = static_cast<char*>(const_cast<void*>(arg->buf)) + (CHUNKSIZE * chnk_ids[i / 2]);
        }
    }
    // setting pointers to the ids and to the chunks
    vector<void*> buf_ptrs(recipient_size * 2);
    for (unsigned long i = 0; i < buf_ptrs.size(); i++) {
        if (i % 2 == 0) // id pointer
            buf_ptrs[i] = &chnk_ids[i / 2];
        else // data pointer
            buf_ptrs[i] = chnks[i / 2];
    }

    // RPC
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_write_data_in_t in{};
    rpc_data_out_t out{};
    int err;
    hg_return_t ret;
    // fill in
    in.path = arg->path.c_str();
    in.offset = arg->in_offset;
    in.updated_size = arg->updated_size;
    if (arg->append)
        in.append = HG_TRUE;
    else
        in.append = HG_FALSE;


    margo_create_wrap(ipc_write_data_id, rpc_write_data_id, arg->path, handle, svr_addr, false);

    auto used_mid = margo_hg_handle_get_instance(handle);

    /* register local target buffer for bulk access */
    ret = margo_bulk_create(used_mid, static_cast<uint32_t>(buf_sizes.size()), buf_ptrs.data(), buf_sizes.data(),
                            HG_BULK_READ_ONLY, &in.bulk_handle);

    if (ret != 0)
        ld_logger->error("{}() failed to create bulk on client", __func__);

    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    auto write_size = static_cast<size_t>(0);
    if (send_ret == HG_SUCCESS) {

        /* decode response */
        ret = margo_get_output(handle, &out);
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
    }
    // Signal calling process that RPC is finished and put written size into return value
    ABT_eventual_set(*(arg->eventual), &write_size, sizeof(write_size));
    margo_destroy(handle);

}

void rpc_send_read_abt(void* _arg) {

    // Prepare buffers
    auto* arg = static_cast<struct read_args*>(_arg);
    auto recipient_size = arg->chnk_ids.size();
    auto chnk_ids = arg->chnk_ids;
    vector<char*> chnks(recipient_size);
    vector<size_t> buf_sizes(recipient_size * 2);

    for (size_t i = 0; i < buf_sizes.size(); i++) {
        // even numbers contain the sizes of ids, while uneven contain the chunksize
        if (i < buf_sizes.size() / 2)
            buf_sizes[i] = sizeof(rpc_chnk_id_t);
        else {

            if (i / 2 == 0) { // First chunk might have an offset
                buf_sizes[i] = CHUNKSIZE - static_cast<unsigned long>(arg->in_offset);
            } else if (i + 1 == buf_sizes.size()) {// if current chunk size is last chunk
                // the last chunk will have the rest of the size, i.e., write size - all applied chunk sizes
                buf_sizes[i] = arg->in_size - (chnk_ids[i / 2] * CHUNKSIZE);
            } else {
                buf_sizes[i] = CHUNKSIZE;
            }
            // position the pointer according to the chunk number
            chnks[i - chnks.size()] = static_cast<char*>(arg->buf) + (CHUNKSIZE * chnk_ids[i - chnk_ids.size()]);
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
    int err;
    hg_return_t ret;
    // fill in
    in.path = arg->path.c_str();
    in.size = arg->in_size;
    in.offset = arg->in_offset;

    // TODO bulk_access on the handler site doesn't work with HG_BULK_READWRITE, This is why we force rpc for now ...
    margo_create_wrap(ipc_read_data_id, rpc_read_data_id, in.path, handle, svr_addr, true);

    auto used_mid = margo_hg_handle_get_instance(handle);
    /* register local target buffer for bulk access */
    ret = margo_bulk_create(used_mid, static_cast<uint32_t>(buf_sizes.size()), buf_ptrs.data(), buf_sizes.data(),
                            HG_BULK_READWRITE, &in.bulk_handle);
    if (ret != 0)
        ld_logger->error("{}() failed to create bulk on client", __func__);

    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    auto read_size = static_cast<size_t>(0);
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = margo_get_output(handle,
                               &out); // XXX handle ret out.res can inidicate a failure with reading on the other side.
        read_size = static_cast<size_t>(out.io_size);
        err = out.res;
        ld_logger->debug("{}() Got response {}", __func__, out.res);
        /* clean up resources consumed by this rpc */
        margo_bulk_free(in.bulk_handle);
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out", __func__);
        err = EAGAIN;
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
int rpc_send_write(const string& path, const size_t in_size, const off_t in_offset, void* buf, size_t& write_size,
                   const bool append, const off_t updated_size) {

    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_write_data_in_t in{};
    rpc_data_out_t out{};
    int err;
    hg_return_t ret;
    // fill in
    in.path = path.c_str();
    in.offset = in_offset;
    in.updated_size = updated_size;
    if (append)
        in.append = HG_TRUE;
    else
        in.append = HG_FALSE;

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