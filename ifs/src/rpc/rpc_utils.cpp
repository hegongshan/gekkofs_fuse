
#include <rpc/rpc_utils.hpp>

hg_bool_t bool_to_merc_bool(const bool state) {
    return state ? static_cast<hg_bool_t>(HG_TRUE) : static_cast<hg_bool_t>(HG_FALSE);
}