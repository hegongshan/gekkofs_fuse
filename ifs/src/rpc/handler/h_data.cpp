
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
    // is write happening over shared memory on the same node?
    auto local_read = is_handle_sm(mid, hgi->addr);
    if (local_read)
        ADAFS_DATA->spdlogger()->debug("Got local read IPC with path {} size {} offset {}", in.path, bulk_size,
                                       in.offset);
    else
        ADAFS_DATA->spdlogger()->debug("Got read RPC with path {} size {} offset {}", in.path, bulk_size, in.offset);

    // set buffer sizes
    vector<hg_size_t> buf_sizes(segment_count);
    size_t chnk_size = 0;
    size_t id_size = 0;
    for (size_t i = 0; i < segment_count; i++) {
        if (i < segment_count / 2) {
            buf_sizes[i] = sizeof(rpc_chnk_id_t);
            id_size += sizeof(rpc_chnk_id_t);
        } else {
            // case for last chunk size
            if ((chnk_size + CHUNKSIZE) > bulk_size)
                buf_sizes[i] = bulk_size - chnk_size - id_size;
            else
                buf_sizes[i] = CHUNKSIZE;
            chnk_size += buf_sizes[i];
        }
    }
    // allocate memory for bulk transfer
    vector<void*> buf_ptrs(segment_count);
    // On a local operation the buffers are allocated in the client on the same node.
    // Hence no memory allocation is necessary
    if (!local_read) {
        for (size_t i = 0; i < segment_count; i++) {
            if (i < segment_count / 2)
                buf_ptrs[i] = new rpc_chnk_id_t;
            else {
                buf_ptrs[i] = new char[buf_sizes[i]];
            }
        }
    }
    if (local_read) {
        // TODO bulk access readwrite doesn't work for some reason ...
        uint32_t actual_count;
        // The data is not transferred. We directly access the data from the client on the same node
        ret = margo_bulk_access(in.bulk_handle, 0, bulk_size, HG_BULK_READWRITE, segment_count, buf_ptrs.data(),
                                buf_sizes.data(), &actual_count);
        if (ret != HG_SUCCESS || segment_count != actual_count)
            ADAFS_DATA->spdlogger()->error("{}() margo_bulk_access failed with ret {}", __func__, ret);
        // read the data
        err = read_chunks(in.path, buf_ptrs, buf_sizes, read_size);

        if (err != 0 || in.size != read_size) {
            out.res = err;
            ADAFS_DATA->spdlogger()->error("{}() Failed to read chunks on path {}", __func__, in.path);
            return rpc_cleanup_respond(&handle, &in, &out, static_cast<hg_bulk_t*>(nullptr));
        }

    } else {
        // create bulk handle for data transfer
        ret = margo_bulk_create(mid, segment_count, buf_ptrs.data(), buf_sizes.data(), HG_BULK_READWRITE, &bulk_handle);

        if (ret != HG_SUCCESS) {
            ADAFS_DATA->spdlogger()->error("{}() Failed to create bulk handle", __func__);
            return rpc_cleanup_respond(&handle, &in, &out, static_cast<hg_bulk_t*>(nullptr));
        }

        // Get the id numbers on the offset 0
        ret = margo_bulk_transfer(mid, HG_BULK_PULL, hgi->addr, in.bulk_handle, 0, bulk_handle, 0, id_size);
        if (ret != HG_SUCCESS) {
            ADAFS_DATA->spdlogger()->error("{}() Failed to fetch data IDs", __func__);
            return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
        }

        // read the data
        err = read_chunks(in.path, buf_ptrs, buf_sizes, read_size);

        if (err != 0 || in.size != read_size) {
            out.res = err;
            ADAFS_DATA->spdlogger()->error("{}() Failed to read chunks on path {}", __func__, in.path);
            return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
        }
        // get the data on the offset after the ids
        ret = margo_bulk_transfer(mid, HG_BULK_PUSH, hgi->addr, in.bulk_handle, id_size, bulk_handle, id_size,
                                  chnk_size);
        if (ret != HG_SUCCESS) {
            ADAFS_DATA->spdlogger()->error("Failed push the data to the client in read operation");
            return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
        }
    }


    out.res = 0;
    out.io_size = read_size;

    ADAFS_DATA->spdlogger()->debug("Sending output response {}", out.res);
    if (bulk_handle)
        return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
    else
        return rpc_cleanup_respond(&handle, &in, &out, static_cast<hg_bulk_t*>(nullptr));
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_read_data)

static hg_return_t rpc_srv_write_data(hg_handle_t handle) {
    rpc_write_data_in_t in{};
    rpc_data_out_t out{};
    hg_bulk_t bulk_handle;

    auto ret = margo_get_input(handle, &in);
    assert(ret == HG_SUCCESS);

    auto hgi = margo_get_info(handle);
    auto mid = margo_hg_info_get_instance(hgi);


    auto segment_count = margo_bulk_get_segment_count(in.bulk_handle);
    auto bulk_size = margo_bulk_get_size(in.bulk_handle);
    // is write happening over shared memory on the same node?
    auto local_write = is_handle_sm(mid, hgi->addr);
    if (local_write)
        ADAFS_DATA->spdlogger()->debug("Got local write IPC with path {} size {} offset {}", in.path, bulk_size,
                                       in.offset);
    else
        ADAFS_DATA->spdlogger()->debug("Got write RPC with path {} size {} offset {}", in.path, bulk_size, in.offset);


    // set buffer sizes
    vector<hg_size_t> buf_sizes(segment_count);
    size_t chnk_size = 0;
    for (size_t i = 0; i < segment_count; i++) {
        if (i % 2 == 0)
            buf_sizes[i] = sizeof(rpc_chnk_id_t);
        else {
            // case for last chunk size
            if ((chnk_size + CHUNKSIZE) > bulk_size)
                buf_sizes[i] = bulk_size - chnk_size;
            else
                buf_sizes[i] = CHUNKSIZE;
        }
        chnk_size += buf_sizes[i];
    }
    // allocate memory for bulk transfer
    vector<void*> buf_ptrs(segment_count);
    // On a local operation the buffers are allocated in the client on the same node.
    // Hence no memory allocation is necessary
    if (!local_write) {
        for (size_t i = 0; i < segment_count; i++) {
            if (i % 2 == 0)
                buf_ptrs[i] = new rpc_chnk_id_t;
            else {
                buf_ptrs[i] = new char[buf_sizes[i]];
            }
        }
    }
    // If local operation the data does not need to be transferred. We just need access to the data ptrs
    if (local_write) {
        uint32_t actual_count;
        // The data is not transferred. We directly access the data from the client on the same node
        ret = margo_bulk_access(in.bulk_handle, 0, bulk_size, HG_BULK_READ_ONLY, segment_count, buf_ptrs.data(),
                                buf_sizes.data(), &actual_count);
        if (ret != HG_SUCCESS || segment_count != actual_count)
            ADAFS_DATA->spdlogger()->error("{}() margo_bulk_access failed with ret {}", __func__, ret);
    } else {
        // create bulk handle
        ret = margo_bulk_create(mid, segment_count, buf_ptrs.data(), buf_sizes.data(), HG_BULK_WRITE_ONLY,
                                &bulk_handle);
        if (ret == HG_SUCCESS) {
            // pull data from client here
            ret = margo_bulk_transfer(mid, HG_BULK_PULL, hgi->addr, in.bulk_handle, 0, bulk_handle, 0, bulk_size);
            if (ret != HG_SUCCESS)
                ADAFS_DATA->spdlogger()->error("Failed to pull data from client in write operation");
            margo_bulk_free(bulk_handle);
        }
    }

    // do write operation if all is good
    if (ret == HG_SUCCESS) {
        out.res = write_chunks(in.path, buf_ptrs, buf_sizes, out.io_size);
        if (out.res != 0) {
            ADAFS_DATA->spdlogger()->error("Failed to write data to local disk.");
            out.io_size = 0;
        }
    } else {
        ADAFS_DATA->spdlogger()->error("Failed to create bulk handle in write operation");
        out.res = EIO;
        out.io_size = 0;
    }

    ADAFS_DATA->spdlogger()->debug("Sending output response {}", out.res);
    auto hret = margo_respond(handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to write request");
    }

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_destroy(handle);

    // free memory in buf_ptrs
    // On a local operation the data is owned by the client who is responsible to free its buffers
    if (!local_write) {
        for (size_t i = 0; i < segment_count; i++) {
            if (i % 2 == 0)
                delete static_cast<rpc_chnk_id_t*>(buf_ptrs[i]);
            else {
                delete[] static_cast<char*>(buf_ptrs[i]);
            }
        }

    }
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_write_data)