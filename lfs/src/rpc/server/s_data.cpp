//
// Created by evie on 7/13/17.
//
#include "../rpc_types.hpp"
#include "../rpc_defs.hpp"

using namespace std;

static hg_return_t rpc_srv_read_data(hg_handle_t handle) {
    rpc_data_in_t in;
    rpc_res_out_t out;
    void* b_buf;
    hg_size_t b_size;
    hg_bulk_t bulk_handle;
    const struct hg_info* hgi;

    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got read RPC with inode {} size {} offset {}", in.inode, in.size, in.offset);
    /* set up target buffer for bulk transfer */
    b_size = 512;
    b_buf = calloc(1, 512);
    sprintf((char*) b_buf, "Hello world!\n");

    hgi = HG_Get_info(handle);

    // do read operation

    ret = HG_Bulk_create(hgi->hg_class, 1, &b_buf, &b_size, HG_BULK_READ_ONLY, &bulk_handle);

    auto mid = margo_hg_class_to_instance(hgi->hg_class);
    // push data to client here
    margo_bulk_transfer(mid, HG_BULK_PUSH, hgi->addr, in.bulk_handle, 0, bulk_handle, 0, b_size);


    out.res = HG_TRUE;
    ADAFS_DATA->spdlogger()->debug("Sending output response {}", out.res);
    auto hret = margo_respond(mid, handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("Failed to respond to read request");
    }

    // Destroy handle when finished
    HG_Free_input(handle, &in);
    HG_Free_output(handle, &out);
    HG_Destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_read_data)

static hg_return_t rpc_srv_write_data(hg_handle_t handle) {
    rpc_data_in_t in;
    rpc_res_out_t out;
    const struct hg_info* hgi;

    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got write RPC with inode {} size {} offset {}", in.inode, in.size, in.offset);

    hgi = HG_Get_info(handle);
    // do write operation

    // TODO pull data from client here

    auto mid = margo_hg_class_to_instance(hgi->hg_class);=

    out.res = HG_TRUE;
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