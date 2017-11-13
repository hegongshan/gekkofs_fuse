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

int ipc_send_unlink(const string& path, const hg_id_t ipc_unlink_id);

#endif //IFS_MARGO_IPC_HPP
