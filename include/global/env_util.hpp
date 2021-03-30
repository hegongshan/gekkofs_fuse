#ifndef GKFS_COMMON_ENV_UTIL_HPP
#define GKFS_COMMON_ENV_UTIL_HPP

#include <string>

namespace gkfs {
namespace env {

std::string
get_var(const std::string& name, const std::string& default_value = "");

} // namespace env
} // namespace gkfs

#endif // GKFS_COMMON_ENV_UTIL_HPP
