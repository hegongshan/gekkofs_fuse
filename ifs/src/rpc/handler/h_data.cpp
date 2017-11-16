
#include <rpc/rpc_types.hpp>
#include <rpc/rpc_defs.hpp>
#include <adafs_ops/data.hpp>

using namespace std;

static hg_return_t rpc_srv_read_data(hg_handle_t handle) {
    rpc_read_data_in_t in{};
    rpc_data_out_t out{};
    void* b_buf;
    int err;
    hg_bulk_t bulk_handle;

    auto ret = margo_get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got read RPC with path {} size {} offset {}", in.path, in.size, in.offset);

    auto hgi = margo_get_info(handle);
    auto mid = margo_hg_info_get_instance(hgi);

    // set up buffer to read
    auto buf = make_unique<char[]>(in.size);

    err = read_file(buf.get(), out.io_size, in.path, in.size, in.offset);

    if (err != 0) {
        ADAFS_DATA->spdlogger()->error("Could not open file with path: {}", in.path);
        out.res = err;
        ADAFS_DATA->spdlogger()->debug("Sending output response {}", out.res);
        auto hret = margo_respond(handle, &out);
        if (hret != HG_SUCCESS) {
            ADAFS_DATA->spdlogger()->error("Failed to respond to read request");
        }
    } else {
        // set up buffer for bulk transfer
        b_buf = (void*) buf.get();

        ret = margo_bulk_create(mid, 1, &b_buf, &in.size, HG_BULK_READ_ONLY, &bulk_handle);

        // push data to client
        if (ret == HG_SUCCESS)
            margo_bulk_transfer(mid, HG_BULK_PUSH, hgi->addr, in.bulk_handle, 0, bulk_handle, 0, in.size);
        else {
            ADAFS_DATA->spdlogger()->error("Failed to send data to client in read operation");
            out.res = EIO;
            out.io_size = 0;
        }
        ADAFS_DATA->spdlogger()->debug("Sending output response {}", out.res);
        // respond rpc
        auto hret = margo_respond(handle, &out);
        if (hret != HG_SUCCESS) {
            ADAFS_DATA->spdlogger()->error("Failed to respond to read request");
        }
        margo_bulk_free(bulk_handle);
    }

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_read_data)

static hg_return_t rpc_srv_write_data(hg_handle_t handle) {
    rpc_write_data_in_t in{};
    rpc_data_out_t out{};
    hg_bulk_t bulk_handle;

    auto ret = margo_get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got write RPC with path {} size {} offset {}", in.path, in.size, in.offset);

    auto hgi = margo_get_info(handle);
    auto mid = margo_hg_info_get_instance(hgi);

    auto segment_count = margo_bulk_get_segment_count(in.bulk_handle);
    auto bulk_size = margo_bulk_get_size(in.bulk_handle);

    // set buffer sizes
    vector<hg_size_t> buf_sizes(segment_count);
    size_t chnk_size = 0;
    for (int i = 0; i < segment_count; i++) {
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
    for (int i = 0; i < segment_count; i++) {
        if (i % 2 == 0)
            buf_ptrs[i] = new rpc_chnk_id_t;
        else {
            buf_ptrs[i] = new char[buf_sizes[i]];
        }
    }
    // create bulk handle
    ret = margo_bulk_create(mid, segment_count, buf_ptrs.data(), buf_sizes.data(), HG_BULK_WRITE_ONLY, &bulk_handle);
    // push data to client
    if (ret == HG_SUCCESS) {
        // pull data from client here TODO use margo_bulk_access for local transfer
        ret = margo_bulk_transfer(mid, HG_BULK_PULL, hgi->addr, in.bulk_handle, 0, bulk_handle, 0, bulk_size);
        if (ret != HG_SUCCESS) {
            ADAFS_DATA->spdlogger()->error("Failed to pull data from client in write operation");
            out.res = EIO;
            out.io_size = 0;
        }

        // do write operation
        out.res = write_chunks(in.path, buf_ptrs, buf_sizes, out.io_size);
        if (out.res != 0) {
            ADAFS_DATA->spdlogger()->error("Failed to write data to local disk.");
            out.io_size = 0;
        }
        margo_bulk_free(bulk_handle);
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
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_write_data)