#ifndef GKFS_DAEMON_ENV
#define GKFS_DAEMON_ENV

#include <config.hpp>

#define ADD_PREFIX(str) DAEMON_ENV_PREFIX str

/* Environment variables for the GekkoFS daemon */
namespace gkfs {
namespace env {

static constexpr auto HOSTS_FILE = ADD_PREFIX("HOSTS_FILE");

} // namespace env
} // namespace gkfs

#undef ADD_PREFIX

#endif // GKFS_DAEMON_ENV
