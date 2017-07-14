//
// Created by evie on 7/13/17.
//

#include "c_data.hpp"
#include "../rpc_types.hpp"

using namespace std;

static int max_retries = 3;

int rpc_send_read(const size_t recipient, const fuse_ino_t inode, const size_t in_size, const off_t in_offset,
                  char* tar_buf) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_data_in_t in;
    rpc_res_out_t out;
    hg_size_t b_size;
    void* b_buf;
    const struct hg_info* hgi;
    // fill in
    in.inode = inode;
    in.size = in_size;
    in.offset = in_offset;
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!RPC_DATA->get_addr_by_hostid(recipient, svr_addr)) {
        ADAFS_DATA->spdlogger()->error("server address not resolvable for host id {}", recipient);
        return 1;
    }
    auto ret = HG_Create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_srv_read_data_id(), &handle);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("creating handle FAILED");
        return 1;
    }

    b_size = 512;
    b_buf = calloc(1, 512);
    /* register local target buffer for bulk access */
    hgi = HG_Get_info(handle);
    ret = HG_Bulk_create(hgi->hg_class, 1, &b_buf, &b_size, HG_BULK_WRITE_ONLY, &in.bulk_handle);
    if (ret != 0)
        ADAFS_DATA->spdlogger()->error("failed to create bulkd on client");

    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(RPC_DATA->client_mid(), handle, &in, 15000);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }

    if (send_ret == HG_SUCCESS) {

        /* decode response */
        ret = HG_Get_output(handle, &out);

        ADAFS_DATA->spdlogger()->debug("Got response mode {}", out.res);
        ADAFS_DATA->spdlogger()->debug("Filled buffer looks like this: {}", (char*) b_buf);
        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        ADAFS_DATA->spdlogger()->error("RPC send_get_attr (timed out)");
    }

    HG_Free_input(handle, &in);
    HG_Destroy(handle);

    return 0;

    return 0;
}

int rpc_send_write() {
    // TODO

    return 0;
}