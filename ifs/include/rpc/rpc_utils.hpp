
#ifndef IFS_RPC_UTILS_HPP
#define IFS_RPC_UTILS_HPP

extern "C" {
#include <mercury_types.h>
#include <mercury_proc_string.h>
#include <margo.h>
}
#include <string>

hg_bool_t bool_to_merc_bool(const bool state);


#endif //IFS_RPC_UTILS_HPP
