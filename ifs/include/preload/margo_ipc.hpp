//
// Created by aelycia on 9/13/17.
//

#ifndef IFS_MARGO_IPC_HPP
#define IFS_MARGO_IPC_HPP

#include <preload/preload.hpp>
#include <preload/ipc_types.hpp>

void send_minimal_rpc(const hg_id_t minimal_id);

int ipc_send_open(const char* path, int flags, const mode_t mode, const hg_id_t ipc_open_id);

#endif //IFS_MARGO_IPC_HPP
