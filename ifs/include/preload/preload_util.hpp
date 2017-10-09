//
// Created by evie on 9/4/17.
//

#ifndef IFS_PRELOAD_UTIL_HPP
#define IFS_PRELOAD_UTIL_HPP

#include "../../configure.hpp"
#include <string>

using namespace std;

bool is_fs_path(const char* path);

// TODO template these two suckers
int db_val_to_stat(const std::string path, std::string db_val, struct stat& attr);

int db_val_to_stat64(const std::string path, std::string db_val, struct stat64& attr);

int getProcIdByName(std::string procName);

#endif //IFS_PRELOAD_UTIL_HPP
