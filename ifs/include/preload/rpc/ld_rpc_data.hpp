//
// Created by evie on 9/8/17.
//

#ifndef IFS_PRELOAD_C_DATA_HPP
#define IFS_PRELOAD_C_DATA_HPP

extern "C" {
#include <abt.h>
#include <abt-snoozer.h>
#include <mercury_types.h>
#include <mercury_proc_string.h>
#include <margo.h>
}

#include <rpc/rpc_types.hpp>
#include <preload/preload.hpp>

#include <iostream>

template<typename T>
int rpc_send_read(const size_t recipient, const std::string& path, const size_t in_size, const off_t in_offset,
                  T* tar_buf, size_t& read_size, const hg_id_t rpc_read_data_id) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_read_data_in_t in;
    rpc_data_out_t out;
    int err;
    // fill in
    in.path = path.c_str();
    in.size = in_size;
    in.offset = in_offset;
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!get_addr_by_hostid(recipient, svr_addr)) {
        LD_LOG_ERROR(debug_fd, "server address not resolvable for host id %lu\n", recipient);
        return 1;
    }
    auto ret = HG_Create(margo_get_context(ld_margo_rpc_id()), svr_addr, rpc_read_data_id, &handle);
    if (ret != HG_SUCCESS) {
        LD_LOG_ERROR0(debug_fd, "creating handle FAILED\n");
        return 1;
    }

    auto hgi = HG_Get_info(handle);
    /* register local target buffer for bulk access */
    auto b_buf = static_cast<void*>(tar_buf);
    ret = HG_Bulk_create(hgi->hg_class, 1, &b_buf, &in_size, HG_BULK_WRITE_ONLY, &in.bulk_handle);
    if (ret != 0)
        LD_LOG_ERROR0(debug_fd, "failed to create bulk on client\n");

    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(ld_margo_rpc_id(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = HG_Get_output(handle,
                            &out); // XXX handle ret out.res can inidicate a failure with reading on the other side.
        tar_buf = static_cast<T*>(b_buf);
        read_size = static_cast<size_t>(out.io_size);
        err = out.res;
        LD_LOG_TRACE(debug_fd, "Got response %d\n", out.res);
        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        LD_LOG_ERROR0(debug_fd, "RPC rpc_send_read (timed out)");
        err = EAGAIN;
    }

    in.path = nullptr;

    HG_Bulk_free(in.bulk_handle);
    HG_Free_input(handle, &in);
    HG_Destroy(handle);

    return err;
}

int rpc_send_write(const size_t recipient, const std::string& path, const size_t in_size, const off_t in_offset,
                   const void* buf, size_t& write_size, const bool append, const hg_id_t rpc_write_data_id);

#endif //IFS_PRELOAD_C_DATA_HPP
