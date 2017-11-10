//
// Created by evie on 6/22/17.
//

#ifndef LFS_RPC_TYPES_HPP
#define LFS_RPC_TYPES_HPP

extern "C" {
#include <mercury_proc_string.h>
#include <margo.h>
}

/* visible API for RPC data types used in RPCS */

MERCURY_GEN_PROC(rpc_minimal_in_t, ((int32_t) (input)))

MERCURY_GEN_PROC(rpc_minimal_out_t, ((int32_t) (output)))
// misc generic rpc types
MERCURY_GEN_PROC(rpc_err_out_t, ((hg_int32_t) (err)))

// Metadentry
MERCURY_GEN_PROC(rpc_create_node_in_t,
                 ((hg_const_string_t) (path))\
((uint32_t) (mode)))

MERCURY_GEN_PROC(rpc_get_attr_in_t, ((hg_const_string_t) (path)))

MERCURY_GEN_PROC(rpc_get_attr_out_t, ((hg_int32_t) (err))
        ((hg_const_string_t) (db_val)))

MERCURY_GEN_PROC(rpc_remove_node_in_t,
                 ((hg_const_string_t) (path)))

MERCURY_GEN_PROC(rpc_update_metadentry_in_t,
                 ((hg_const_string_t) (path))\
((uint64_t) (nlink))\
((hg_uint32_t) (mode))\
((hg_uint32_t) (uid))\
((hg_uint32_t) (gid))\
((hg_int64_t) (size))\
((hg_uint64_t) (inode_no))\
((hg_int64_t) (blocks))\
((hg_int64_t) (atime))\
((hg_int64_t) (mtime))\
((hg_int64_t) (ctime))\
((hg_bool_t) (nlink_flag))\
((hg_bool_t) (mode_flag))\
((hg_bool_t) (uid_flag))\
((hg_bool_t) (gid_flag))\
((hg_bool_t) (size_flag))\
((hg_bool_t) (inode_no_flag))\
((hg_bool_t) (block_flag))\
((hg_bool_t) (atime_flag))\
((hg_bool_t) (mtime_flag))\
((hg_bool_t) (ctime_flag)))

MERCURY_GEN_PROC(rpc_update_metadentry_size_in_t, ((hg_const_string_t) (path))
        ((hg_int64_t) (size))
        ((hg_bool_t) (append)))

MERCURY_GEN_PROC(rpc_update_metadentry_size_out_t, ((hg_int32_t) (err))
        ((hg_int64_t) (ret_size)))

// data
MERCURY_GEN_PROC(rpc_read_data_in_t,
                 ((hg_const_string_t) (path))\
((hg_size_t) (size))\
((int64_t) (offset))\
((hg_bulk_t) (bulk_handle)))

MERCURY_GEN_PROC(rpc_data_out_t,
                 ((int32_t) (res))\
((hg_size_t) (io_size)))

MERCURY_GEN_PROC(rpc_write_data_in_t,
                 ((hg_const_string_t) (path))\
((hg_size_t) (size))\
((int64_t) (offset))\
((hg_bool_t) (append))\
((hg_bulk_t) (bulk_handle))\
((hg_int64_t) (updated_size)))



/** OLD BELOW
// create dentry
MERCURY_GEN_PROC(rpc_create_dentry_in_t,
                 ((uint64_t) (parent_inode))\
((hg_const_string_t) (filename))\
((uint32_t) (mode)))

MERCURY_GEN_PROC(rpc_create_dentry_out_t, ((uint64_t) (inode)))
// create mdata
MERCURY_GEN_PROC(rpc_create_mdata_in_t,
                 ((uint64_t) (inode))\
((uint32_t) (gid))\
((uint32_t) (uid))\
((uint32_t) (mode)))

MERCURY_GEN_PROC(rpc_create_mdata_out_t, ((hg_bool_t) (success)))
// get_attr
MERCURY_GEN_PROC(rpc_get_attr_in_t, ((uint64_t) (inode)))

MERCURY_GEN_PROC(rpc_get_attr_out_t,
                 ((uint64_t) (nlink))\
((uint32_t) (mode))\
((uint32_t) (uid))\
((uint32_t) (gid))\
((int64_t) (size))\
((int64_t) (blocks))\
((int64_t) (atime))\
((int64_t) (mtime))\
((int64_t) (ctime)))
// lookup
MERCURY_GEN_PROC(rpc_lookup_in_t,
                 ((uint64_t) (parent_inode))\
((hg_const_string_t) (filename)))

MERCURY_GEN_PROC(rpc_lookup_out_t, ((uint64_t) (inode)))
// remove dentry
MERCURY_GEN_PROC(rpc_remove_dentry_in_t,
                 ((uint64_t) (parent_inode))\
((hg_const_string_t) (filename)))

MERCURY_GEN_PROC(rpc_remove_dentry_out_t, ((uint64_t) (del_inode)))
// remove mdata
MERCURY_GEN_PROC(rpc_remove_mdata_in_t, ((uint64_t) (del_inode)))

MERCURY_GEN_PROC(rpc_remove_mdata_out_t, ((hg_bool_t) (success)))
// data
MERCURY_GEN_PROC(rpc_read_data_in_t,
                 ((uint64_t) (inode))\
((hg_size_t) (size))\
((int64_t) (offset))\
((hg_bulk_t) (bulk_handle)))

MERCURY_GEN_PROC(rpc_data_out_t,
                 ((int32_t) (res))\
                         ((hg_size_t) (io_size)))

MERCURY_GEN_PROC(rpc_write_data_in_t,
                 ((uint64_t) (inode))\
((hg_size_t) (size))\
((int64_t) (offset))\
((hg_bool_t) (append))\
((hg_bulk_t) (bulk_handle)))
 */

#endif //LFS_RPC_TYPES_HPP
