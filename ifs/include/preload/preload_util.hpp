//
// Created by evie on 9/4/17.
//

#ifndef IFS_PRELOAD_UTIL_HPP
#define IFS_PRELOAD_UTIL_HPP

#include "../../configure.hpp"
#include <string>

using namespace std;

static const std::string mountdir = ADAFS_MOUNTDIR;

bool is_fs_path(const char* path);

#endif //IFS_PRELOAD_UTIL_HPP
