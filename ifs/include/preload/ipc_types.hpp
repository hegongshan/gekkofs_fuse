//
// Created by evie on 9/12/17.
//

#ifndef IFS_IPC_TYPES_HPP
#define IFS_IPC_TYPES_HPP

extern "C" {
#include <mercury_types.h>
}

MERCURY_GEN_PROC(ipc_res_out_t, ((hg_bool_t) (res))) // generic return type


MERCURY_GEN_PROC(ipc_open_in_t, ((hg_string_t) (path))
        ((hg_int32_t) (flags)) \
((hg_uint32_t) (mode)))

#endif //IFS_IPC_TYPES_HPP
