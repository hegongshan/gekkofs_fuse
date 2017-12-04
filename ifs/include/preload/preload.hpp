
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

#include "../../configure.hpp"
#include <global_defs.hpp>

extern "C" {
#include <abt.h>
#include <mercury.h>
#include <margo.h>
}

#include <preload/preload_util.hpp>

#define EUNKNOWN (-1)

bool ld_is_env_initialized();

void init_preload() __attribute__((constructor));

void destroy_preload() __attribute__((destructor));

#endif //IOINTERCEPT_PRELOAD_HPP
