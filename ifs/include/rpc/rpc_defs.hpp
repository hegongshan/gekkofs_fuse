//
// Created by evie on 6/22/17.
//

#ifndef LFS_RPC_DEFS_HPP
#define LFS_RPC_DEFS_HPP

#include "../../main.hpp"

/* visible API for RPC operations */

DECLARE_MARGO_RPC_HANDLER(rpc_minimal)

DECLARE_MARGO_RPC_HANDLER(ipc_srv_fs_config)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_create_node)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_attr)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_remove_node)

// data
DECLARE_MARGO_RPC_HANDLER(rpc_srv_read_data)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_write_data)

// IPCs
DECLARE_MARGO_RPC_HANDLER(ipc_srv_open)

DECLARE_MARGO_RPC_HANDLER(ipc_srv_stat)

DECLARE_MARGO_RPC_HANDLER(ipc_srv_unlink)

/** OLD BELOW
// mdata ops
DECLARE_MARGO_RPC_HANDLER(rpc_srv_create_mdata)
DECLARE_MARGO_RPC_HANDLER(rpc_srv_attr)
DECLARE_MARGO_RPC_HANDLER(rpc_srv_remove_mdata)

// dentry ops
DECLARE_MARGO_RPC_HANDLER(rpc_srv_lookup)
DECLARE_MARGO_RPC_HANDLER(rpc_srv_create_dentry)
DECLARE_MARGO_RPC_HANDLER(rpc_srv_remove_dentry)

// data
DECLARE_MARGO_RPC_HANDLER(rpc_srv_read_data)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_write_data)
*/


#endif //LFS_RPC_DEFS_HPP
