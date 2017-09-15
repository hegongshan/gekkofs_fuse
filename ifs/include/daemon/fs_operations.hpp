//
// Created by evie on 9/4/17.
//

#ifndef IFS_FS_OPERATIONS_HPP
#define IFS_FS_OPERATIONS_HPP

#include "../../main.hpp"

int adafs_open(std::string& path, int flags, mode_t mode);

FILE* adafs_fopen(std::string& path, const char* mode);

int adafs_unlink(std::string& path);

int adafs_close(std::string& path);

int adafs_stat(std::string& path, struct stat* buf);

ssize_t adafs_write(std::string& path, char* buf, size_t size, off_t offset);

ssize_t adafs_read(std::string& path, char* buf, size_t size, off_t offset);


#endif //IFS_FS_OPERATIONS_HPP
