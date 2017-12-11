
#ifndef IFS_PRELOAD_C_DATA_WS_HPP
#define IFS_PRELOAD_C_DATA_WS_HPP

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

struct write_args {
    std::shared_ptr<std::string> path;
    size_t in_size;
    off_t in_offset;
    const void* buf;
    size_t chnk_start;
    off_t updated_size;
    std::vector<unsigned long>* chnk_ids;
    size_t recipient;
    ABT_eventual* eventual;
};

struct read_args {
    std::shared_ptr<std::string> path;
    size_t in_size;
    off_t in_offset;
    void* buf;
    std::vector<unsigned long>* chnk_ids;
    size_t recipient;
    ABT_eventual* eventual;
};

void rpc_send_write_abt(void* _arg);

void rpc_send_read_abt(void* _arg);


template<typename T>
int rpc_send_read(const std::string& path, const size_t in_size, const off_t in_offset, T* tar_buf, size_t& read_size) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_read_data_in_t in{};
    rpc_data_out_t out{};
    int err;
    hg_return_t ret;
    // fill in
    in.path = path.c_str();
    in.size = in_size;
    in.offset = in_offset;

    margo_create_wrap(ipc_read_data_id, rpc_read_data_id, path, handle, svr_addr, false);

    auto used_mid = margo_hg_handle_get_instance(handle);
    /* register local target buffer for bulk access */
    auto b_buf = static_cast<void*>(tar_buf);
    ret = margo_bulk_create(used_mid, 1, &b_buf, &in_size, HG_BULK_WRITE_ONLY, &in.bulk_handle);
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
        ret = margo_get_output(handle,
                               &out); // XXX handle ret out.res can inidicate a failure with reading on the other side.
        tar_buf = static_cast<T*>(b_buf);
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

    margo_destroy(handle);

    return err;
}

int rpc_send_write(const std::string& path, size_t in_size, off_t in_offset, void* buf, size_t& write_size,
                   bool append, off_t updated_size);

#endif //IFS_PRELOAD_C_DATA_WS_HPP
