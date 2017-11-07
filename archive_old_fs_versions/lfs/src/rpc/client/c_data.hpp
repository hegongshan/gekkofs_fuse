//
// Created by evie on 7/13/17.
//

#ifndef LFS_C_DATA_HPP
#define LFS_C_DATA_HPP

#include "../../main.hpp"
#include "../rpc_types.hpp"

static int max_retries = 3;

template<typename T>
int rpc_send_read(const size_t recipient, const fuse_ino_t inode, const size_t in_size, const off_t in_offset,
                  T* tar_buf, size_t& read_size) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_read_data_in_t in;
    rpc_data_out_t out;
    int err;
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

    auto b_buf = static_cast<void*>(tar_buf);
    /* register local target buffer for bulk access */
    auto hgi = HG_Get_info(handle);
    ret = HG_Bulk_create(hgi->hg_class, 1, &b_buf, &in_size, HG_BULK_WRITE_ONLY, &in.bulk_handle);
    if (ret != 0)
        ADAFS_DATA->spdlogger()->error("failed to create bulkd on client");

    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(RPC_DATA->client_mid(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = HG_Get_output(handle, &out);
        tar_buf = static_cast<T*>(b_buf);
        read_size = static_cast<size_t>(out.io_size);
        err = out.res;
        ADAFS_DATA->spdlogger()->debug("Got response {}", out.res);
        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        ADAFS_DATA->spdlogger()->error("RPC rpc_send_read (timed out)");
        err = EAGAIN;
    }

    HG_Bulk_free(in.bulk_handle);
    HG_Free_input(handle, &in);
    HG_Destroy(handle);

    return err;
}

int rpc_send_write(const size_t recipient, const fuse_ino_t inode, const size_t in_size, const off_t in_offset,
                   const char *buf, size_t &write_size, const bool append);

#endif //LFS_C_DATA_HPP
