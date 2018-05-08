
#ifndef IOINTERCEPT_PRELOAD_HPP
#define IOINTERCEPT_PRELOAD_HPP

#include <memory>
#include <map>
#include <mutex>
#include <iostream>
#include <sys/statfs.h>
#include <cstdio>
#include <cstdint>
#include <fcntl.h>
#include <cerrno>
#include <unistd.h>

extern "C" {
#include <abt.h>
#include <mercury.h>
#include <margo.h>
}

#include <preload/preload_util.hpp>
#include <preload/preload_context.hpp>

#define EUNKNOWN (-1)

#define CTX PreloadContext::getInstance()

bool ld_is_aux_loaded();

void init_ld_env_if_needed();

void init_preload() __attribute__((constructor));

void destroy_preload() __attribute__((destructor));

#endif //IOINTERCEPT_PRELOAD_HPP
