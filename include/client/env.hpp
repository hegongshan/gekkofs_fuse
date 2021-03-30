#ifndef GKFS_CLIENT_ENV
#define GKFS_CLIENT_ENV

#include <config.hpp>

#define ADD_PREFIX(str) CLIENT_ENV_PREFIX str

/* Environment variables for the GekkoFS client */
namespace gkfs::env {

static constexpr auto LOG = ADD_PREFIX("LOG");

#ifdef GKFS_DEBUG_BUILD
static constexpr auto LOG_DEBUG_VERBOSITY = ADD_PREFIX("LOG_DEBUG_VERBOSITY");
static constexpr auto LOG_SYSCALL_FILTER = ADD_PREFIX("LOG_SYSCALL_FILTER");
#endif

static constexpr auto LOG_OUTPUT = ADD_PREFIX("LOG_OUTPUT");
static constexpr auto LOG_OUTPUT_TRUNC = ADD_PREFIX("LOG_OUTPUT_TRUNC");
static constexpr auto CWD = ADD_PREFIX("CWD");
static constexpr auto HOSTS_FILE = ADD_PREFIX("HOSTS_FILE");
#ifdef GKFS_ENABLE_FORWARDING
static constexpr auto FORWARDING_MAP_FILE = ADD_PREFIX("FORWARDING_MAP_FILE");
#endif

} // namespace gkfs::env

#undef ADD_PREFIX

#endif // GKFS_CLIENT_ENV
