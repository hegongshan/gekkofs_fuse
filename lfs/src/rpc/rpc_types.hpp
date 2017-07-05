//
// Created by evie on 6/22/17.
//

#ifndef LFS_RPC_TYPES_HPP
#define LFS_RPC_TYPES_HPP


#include "../main.hpp"

/* visible API for RPC data types used in RPCS */

MERCURY_GEN_PROC(rpc_minimal_in_t, ((int32_t) (input)))

MERCURY_GEN_PROC(rpc_minimal_out_t, ((int32_t) (output)))
// create
MERCURY_GEN_PROC(rpc_create_in_t,
                 ((uint64_t) (parent_inode))\
((hg_const_string_t) (filename))\
((uint32_t) (gid))\
((uint32_t) (uid))\
((uint32_t) (mode)))
MERCURY_GEN_PROC(rpc_create_out_t, ((uint64_t) (new_inode)))
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


#endif //LFS_RPC_TYPES_HPP
