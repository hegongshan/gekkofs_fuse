//
// Created by aelycia on 9/13/17.
//

#ifndef IFS_MARGO_IPC_HPP
#define IFS_MARGO_IPC_HPP

#include <preload/preload.hpp>
#include <preload/ipc_types.hpp>

void send_minimal_ipc(const hg_id_t minimal_id);

bool ipc_send_get_fs_config(const hg_id_t ipc_get_config_id);

int ipc_send_open(const char* path, int flags, const mode_t mode, const hg_id_t ipc_open_id);

int ipc_send_stat(const char* path, std::string& attr, const hg_id_t ipc_stat_id);

int ipc_send_unlink(const char* path, const hg_id_t ipc_unlink_id);

#endif //IFS_MARGO_IPC_HPP
