#ifndef GEKKOFS_GLOBAL_RPC_UTILS_HPP
#define GEKKOFS_GLOBAL_RPC_UTILS_HPP

extern "C" {
#include <mercury_types.h>
#include <mercury_proc_string.h>
}

#include <string>

namespace gkfs::rpc {

hg_bool_t
bool_to_merc_bool(bool state);

std::string
get_my_hostname(bool short_hostname = false);

std::string
get_host_by_name(const std::string& hostname);

} // namespace gkfs::rpc

#endif // GEKKOFS_GLOBAL_RPC_UTILS_HPP
