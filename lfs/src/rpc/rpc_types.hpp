//
// Created by evie on 6/22/17.
//

#ifndef LFS_RPC_TYPES_HPP
#define LFS_RPC_TYPES_HPP

#include "../main.hpp"

/* visible API for RPC data types used in RPCS */

MERCURY_GEN_PROC(rpc_minimal_in_t, ((int32_t) (input)))

MERCURY_GEN_PROC(rpc_minimal_out_t, ((int32_t) (output)))

MERCURY_GEN_PROC(rpc_create_in_t, (
        (uint64_t) (parent_inode)
                (char)(filename)
                (uint32_t)(gid)
                (uint32_t)(uid)
                (uint32_t)(mode)
))


#endif //LFS_RPC_TYPES_HPP
