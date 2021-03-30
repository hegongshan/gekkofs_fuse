#ifndef GKFS_DAEMON_RPC_DEFS_HPP
#define GKFS_DAEMON_RPC_DEFS_HPP

extern "C" {
#include <margo.h>
}

/* visible API for RPC operations */

DECLARE_MARGO_RPC_HANDLER(rpc_srv_get_fs_config)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_create)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_stat)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_decr_size)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_remove_metadata)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_update_metadentry)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_get_metadentry_size)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_update_metadentry_size)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_get_dirents)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_get_dirents_extended)
#ifdef HAS_SYMLINKS

DECLARE_MARGO_RPC_HANDLER(rpc_srv_mk_symlink)

#endif


// data
DECLARE_MARGO_RPC_HANDLER(rpc_srv_remove_data)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_read)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_write)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_truncate)

DECLARE_MARGO_RPC_HANDLER(rpc_srv_get_chunk_stat)

#endif // GKFS_DAEMON_RPC_DEFS_HPP
