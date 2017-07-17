//
// Created by evie on 7/13/17.
//
#include "../rpc_types.hpp"
#include "../rpc_defs.hpp"
#include "../../adafs_ops/io.hpp"

using namespace std;

static hg_return_t rpc_srv_read_data(hg_handle_t handle) {
    rpc_read_data_in_t in;
    rpc_data_out_t out;
    void* b_buf;
    int err;
    hg_bulk_t bulk_handle;

    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got read RPC with inode {} size {} offset {}", in.inode, in.size, in.offset);

    auto hgi = HG_Get_info(handle);
    auto mid = margo_hg_class_to_instance(hgi->hg_class);

    // set up buffer to read
    auto buf = make_unique<char[]>(in.size);

    // do read operation
    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= fmt::FormatInt(in.inode).c_str();
    chnk_path /= "data"s;

    err = read_file(buf.get(), out.io_size, chnk_path.c_str(), in.size, in.offset);

    if (err != 0) {
        ADAFS_DATA->spdlogger()->error("Could not open file with inode: {}", in.inode);
        out.res = err;
        ADAFS_DATA->spdlogger()->debug("Sending output response {}", out.res);
        auto hret = margo_respond(mid, handle, &out);
        if (hret != HG_SUCCESS) {
            ADAFS_DATA->spdlogger()->error("Failed to respond to read request");
        }
    } else {
        // set up buffer for bulk transfer
        b_buf = (void*) buf.get();

        ret = HG_Bulk_create(hgi->hg_class, 1, &b_buf, &in.size, HG_BULK_READ_ONLY, &bulk_handle);

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
        auto hret = margo_respond(mid, handle, &out);
        if (hret != HG_SUCCESS) {
            ADAFS_DATA->spdlogger()->error("Failed to respond to read request");
        }
        HG_Bulk_free(bulk_handle);
    }

    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_read_data)

static hg_return_t rpc_srv_write_data(hg_handle_t handle) {
    rpc_write_data_in_t in;
    rpc_data_out_t out;
    void *b_buf;
    hg_bulk_t bulk_handle;

    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got write RPC with inode {} size {} offset {}", in.inode, in.size, in.offset);

    auto hgi = HG_Get_info(handle);
    auto mid = margo_hg_class_to_instance(hgi->hg_class);
    // register local buffer to fill for bulk pull
    auto b_buf_wrap = make_unique<char[]>(in.size);
    b_buf = static_cast<void *>(b_buf_wrap.get());
    ret = HG_Bulk_create(hgi->hg_class, 1, &b_buf, &in.size, HG_BULK_WRITE_ONLY, &bulk_handle);
    // push data to client
    if (ret == HG_SUCCESS) {
        // pull data from client here
        margo_bulk_transfer(mid, HG_BULK_PULL, hgi->addr, in.bulk_handle, 0, bulk_handle, 0, in.size);
        // do write operation
        auto buf = static_cast<char *>(b_buf);
        out.res = write_file(in.inode, buf, out.io_size, in.size, in.offset, (in.append == HG_TRUE));
        if (out.res != 0) {
            ADAFS_DATA->spdlogger()->error("Failed to write data to local disk.");
            out.io_size = 0;
        }
        HG_Bulk_free(bulk_handle);
    } else {
        ADAFS_DATA->spdlogger()->error("Failed to pull data from client in write operation");
        out.res = EIO;
        out.io_size = 0;
    }
    ADAFS_DATA->spdlogger()->debug("Sending output response {}", out.res);
    auto hret = margo_respond(mid, handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to write request");
    }

    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_write_data)