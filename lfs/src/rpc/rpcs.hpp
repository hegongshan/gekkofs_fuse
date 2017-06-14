//
// Created by evie on 6/14/17.
//

#ifndef LFS_RPCS_HPP
#define LFS_RPCS_HPP

#include "../main.hpp"

/* visible API for example RPC operation */

MERCURY_GEN_PROC(my_rpc_out_t, ((int32_t) (ret)))

MERCURY_GEN_PROC(my_rpc_in_t,
                 ((int32_t) (input_val))\
((hg_bulk_t) (bulk_handle)))

DECLARE_MARGO_RPC_HANDLER(my_rpc_ult)

DECLARE_MARGO_RPC_HANDLER(my_rpc_shutdown_ult)

#endif //LFS_RPCS_HPP
