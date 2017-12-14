
#include <rpc/rpc_types.hpp>
#include <rpc/rpc_defs.hpp>
#include <adafs_ops/data.hpp>
#include <rpc/rpc_utils.hpp>

using namespace std;

static hg_return_t rpc_srv_read_data(hg_handle_t handle) {
    rpc_read_data_in_t in{};
    rpc_data_out_t out{};
    int err;
    hg_bulk_t bulk_handle = nullptr;
    auto read_size = static_cast<size_t>(0);
    // Set default out for error
    out.res = EIO;
    out.io_size = 0;

    auto ret = margo_get_input(handle, &in);
    assert(ret == HG_SUCCESS);

    auto hgi = margo_get_info(handle);
    auto mid = margo_hg_info_get_instance(hgi);


    auto segment_count = margo_bulk_get_segment_count(in.bulk_handle);
    auto bulk_size = margo_bulk_get_size(in.bulk_handle);
    ADAFS_DATA->spdlogger()->debug("{}() Got read RPC (local {}) with path {} size {} offset {}", __func__,
                                   (margo_get_info(handle)->target_id == ADAFS_DATA->host_id()), in.path, bulk_size,
                                   in.offset);

    // set buffer sizes
    vector<hg_size_t> buf_sizes(segment_count);
    size_t buf_size = 0;
    size_t id_size = 0;
    for (size_t i = 0; i < segment_count; i++) {
        if (i < segment_count / 2) {
            buf_sizes[i] = sizeof(rpc_chnk_id_t);
            id_size += sizeof(rpc_chnk_id_t);
        } else {
            if (i == segment_count / 2) { // first chunk which might have an offset
                if (in.size + in.offset < CHUNKSIZE)
                    buf_sizes[i] = static_cast<size_t>(in.size);
                else if (in.offset > 0) // if the first chunk is the very first chunk in the buffer
                    buf_sizes[i] = static_cast<size_t>(CHUNKSIZE - in.offset);
                else
                    buf_sizes[i] = CHUNKSIZE;
            } else if (i + 1 == buf_sizes.size()) {// last chunk has remaining size
                buf_sizes[i] = in.size - buf_size;
            } else {
                buf_sizes[i] = CHUNKSIZE;
            }
            buf_size += buf_sizes[i];
        }
    }
    // array of pointers for bulk transfer (allocated in bulk_create)
    vector<void*> buf_ptrs(segment_count);
    // create bulk handle for data transfer (buffers are allocated internally)
    ret = margo_bulk_create(mid, segment_count, nullptr, buf_sizes.data(), HG_BULK_READWRITE, &bulk_handle);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to create bulk handle", __func__);
        return rpc_cleanup_respond(&handle, &in, &out, static_cast<hg_bulk_t*>(nullptr));
    }
    // access the internally allocated memory buffer and put it into buf_ptrs
    uint32_t actual_count;
    ret = margo_bulk_access(bulk_handle, 0, bulk_size, HG_BULK_READWRITE, segment_count, buf_ptrs.data(),
                            buf_sizes.data(), &actual_count);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to access allocated memory segments for bulk transfer", __func__);
        return rpc_cleanup_respond(&handle, &in, &out, static_cast<hg_bulk_t*>(nullptr));
    }

    // Get the id numbers on the offset 0
    ret = margo_bulk_transfer(mid, HG_BULK_PULL, hgi->addr, in.bulk_handle, 0, bulk_handle, 0, id_size);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to fetch data IDs", __func__);
        return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
    }

    // read the data
    err = read_chunks(in.path, in.offset, buf_ptrs, buf_sizes, read_size);

    if (err != 0 || in.size != read_size) {
        out.res = err;
        ADAFS_DATA->spdlogger()->error("{}() Failed to read chunks on path {}", __func__, in.path);
        return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
    }
    // get the data on the offset after the ids
    ret = margo_bulk_transfer(mid, HG_BULK_PUSH, hgi->addr, in.bulk_handle, id_size, bulk_handle, id_size,
                              buf_size);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed push the data to the client in read operation", __func__);
        return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
    }

    out.res = 0;
    out.io_size = read_size;

    //cleanup
    ADAFS_DATA->spdlogger()->debug("{}() Sending output response {}", __func__, out.res);
    ret = rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);

    return ret;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_read_data)

static hg_return_t rpc_srv_write_data(hg_handle_t handle) {
    rpc_write_data_in_t in{};
    rpc_data_out_t out{};
    hg_bulk_t bulk_handle = nullptr;
    // default out
    out.res = EIO;
    out.io_size = 0;

    auto ret = margo_get_input(handle, &in);
    assert(ret == HG_SUCCESS);

    auto hgi = margo_get_info(handle);
    auto mid = margo_hg_info_get_instance(hgi);


    auto segment_count = margo_bulk_get_segment_count(in.bulk_handle);
    auto bulk_size = margo_bulk_get_size(in.bulk_handle);
    ADAFS_DATA->spdlogger()->debug("{}() Got write RPC (local {}) with path {} size {} offset {}", __func__,
                                   (margo_get_info(handle)->target_id == ADAFS_DATA->host_id()), in.path, bulk_size,
                                   in.offset);


    // set buffer sizes information
    vector<hg_size_t> buf_sizes(segment_count);
    size_t chnk_size = 0;
    size_t id_size = 0;
    for (size_t i = 0; i < segment_count; i++) {
        if (i < segment_count / 2) {
            buf_sizes[i] = sizeof(rpc_chnk_id_t);
            id_size += sizeof(rpc_chnk_id_t);

        } else {
            if (i == segment_count / 2) { // first chunk
                buf_sizes[i] = static_cast<unsigned long>(CHUNKSIZE - in.offset);
            } else if ((chnk_size + CHUNKSIZE + id_size) > bulk_size) // last chunk
                buf_sizes[i] = bulk_size - chnk_size - id_size;
            else
                buf_sizes[i] = CHUNKSIZE;
            chnk_size += buf_sizes[i];
        }
    }
    // array of pointers for bulk transfer (allocated in margo_bulk_create)
    vector<void*> buf_ptrs(segment_count);
    // create bulk handle and allocated memory for buffer with buf_sizes information
    ret = margo_bulk_create(mid, segment_count, nullptr, buf_sizes.data(), HG_BULK_WRITE_ONLY, &bulk_handle);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to create bulk handle", __func__);
        return rpc_cleanup_respond(&handle, &in, &out, static_cast<hg_bulk_t*>(nullptr));
    }
    // access the internally allocated memory buffer and put it into buf_ptrs
    uint32_t actual_count;
    ret = margo_bulk_access(bulk_handle, 0, bulk_size, HG_BULK_READWRITE, segment_count, buf_ptrs.data(),
                            buf_sizes.data(), &actual_count);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to access allocated buffer from bulk handle", __func__);
        return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
    }
    // pull data from client here
    ret = margo_bulk_transfer(mid, HG_BULK_PULL, hgi->addr, in.bulk_handle, 0, bulk_handle, 0, bulk_size);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to pull data from client", __func__);
        return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
    }

    // do write operation if all is good
    out.res = write_chunks(in.path, buf_ptrs, buf_sizes, in.offset, out.io_size);
    if (out.res != 0) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to write data to local disk.");
        return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
    }
    // respond and cleanup
    ADAFS_DATA->spdlogger()->debug("{}() Sending output response {}", __func__, out.res);
    ret = rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);

    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_write_data)