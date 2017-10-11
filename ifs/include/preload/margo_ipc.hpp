//
// Created by aelycia on 9/13/17.
//

#ifndef IFS_MARGO_IPC_HPP
#define IFS_MARGO_IPC_HPP

#include <preload/preload.hpp>
#include <preload/ipc_types.hpp>

void send_minimal_ipc(const hg_id_t minimal_id);

bool ipc_send_get_fs_config(const hg_id_t ipc_get_config_id);

int ipc_send_open(const string& path, int flags, const mode_t mode, const hg_id_t ipc_open_id);

int ipc_send_stat(const string& path, std::string& attr, const hg_id_t ipc_stat_id);

int ipc_send_unlink(const string& path, const hg_id_t ipc_unlink_id);

int ipc_send_write(const string& path, const size_t in_size, const off_t in_offset,
                   const void* buf, size_t& write_size, const bool append, const hg_id_t ipc_write_id);

#endif //IFS_MARGO_IPC_HPP
