#ifndef IOINTERCEPT_PRELOAD_HPP
#define IOINTERCEPT_PRELOAD_HPP

#include <client/preload_context.hpp>

#define EUNKNOWN (-1)

#define CTX gkfs::preload::PreloadContext::getInstance()
namespace gkfs::preload {
void
init_ld_env_if_needed();
} // namespace gkfs::preload

void
init_preload() __attribute__((constructor));

void
destroy_preload() __attribute__((destructor));


#endif // IOINTERCEPT_PRELOAD_HPP
