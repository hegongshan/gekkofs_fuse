//
// Created by evie on 9/12/17.
//

#ifndef IFS_IPC_TYPES_HPP
#define IFS_IPC_TYPES_HPP


#include <preload/preload.hpp>

MERCURY_GEN_PROC(ipc_res_out_t, ((hg_bool_t) (res))) // generic return type

MERCURY_GEN_PROC(rpc_minimal_in_tt, ((int32_t) (input)))

MERCURY_GEN_PROC(rpc_minimal_out_tt, ((int32_t) (output)))

MERCURY_GEN_PROC(ipc_open_in_t, ((hg_const_string_t) (path))
        ((hg_int32_t) (flags)) \
((hg_uint32_t) (mode)))

MERCURY_GEN_PROC(ipc_unlink_in_t, ((hg_const_string_t) (path)))

#endif //IFS_IPC_TYPES_HPP
