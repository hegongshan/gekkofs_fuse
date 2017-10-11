//
// Created by aelycia on 9/13/17.
//

#ifndef IFS_MARGO_IPC_HPP
#define IFS_MARGO_IPC_HPP

#include <preload/preload.hpp>
#include <preload/ipc_types.hpp>
#include <rpc/rpc_types.hpp>

using namespace std;


void send_minimal_ipc(const hg_id_t minimal_id);

bool ipc_send_get_fs_config(const hg_id_t ipc_get_config_id);

int ipc_send_open(const string& path, int flags, const mode_t mode, const hg_id_t ipc_open_id);

int ipc_send_stat(const string& path, std::string& attr, const hg_id_t ipc_stat_id);

int ipc_send_unlink(const string& path, const hg_id_t ipc_unlink_id);

int ipc_send_write(const string& path, const size_t in_size, const off_t in_offset,
                   const void* buf, size_t& write_size, const bool append, const hg_id_t ipc_write_id);

template<typename T>
int ipc_send_read(const string& path, const size_t in_size, const off_t in_offset,
                  T* tar_buf, size_t& read_size, const hg_id_t ipc_read_id) {

    hg_handle_t handle;
    ipc_read_data_in_t in;
    ipc_data_out_t out;
    int err;
    // fill in
    in.path = path.c_str();
    in.size = in_size;
    in.offset = in_offset;

    auto ret = HG_Create(margo_get_context(ld_margo_ipc_id()), daemon_addr(), ipc_read_id, &handle);
    if (ret != HG_SUCCESS) {
        LD_LOG_ERROR0(debug_fd, "creating handle FAILED\n");
        return 1;
    }

    auto hgi = HG_Get_info(handle);
    /* register local target buffer for bulk access */
    auto b_buf = static_cast<void*>(tar_buf);
    ret = HG_Bulk_create(hgi->hg_class, 1, &b_buf, &in_size, HG_BULK_WRITE_ONLY, &in.bulk_handle);
    if (ret != 0) {
        LD_LOG_ERROR0(debug_fd, "failed to create bulkd on client when reading\n");
        return 1;
    }

    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(ld_margo_ipc_id(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = HG_Get_output(handle,
                            &out); // XXX handle ret out.res can inidicate a failure with reading on the other side.
        tar_buf = static_cast<T*>(b_buf); // XXX wtf am I doing here?
        read_size = static_cast<size_t>(out.io_size);
        err = out.res;
        LD_LOG_DEBUG(debug_fd, "Got response %d\n", out.res);

        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        LD_LOG_ERROR0(debug_fd, "ipc_send_read (timed out)\n");
        err = EAGAIN;
    }

    in.path = nullptr;

    HG_Bulk_free(in.bulk_handle);
    HG_Free_input(handle, &in);
    HG_Destroy(handle);

    return err;
}

#endif //IFS_MARGO_IPC_HPP
