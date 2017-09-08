//
// Created by evie on 9/4/17.
//

#ifndef IFS_FS_OPERATIONS_HPP
#define IFS_FS_OPERATIONS_HPP

#include "../../main.hpp"

int adafs_open(char* path, int flags, mode_t mode);

FILE* adafs_fopen(char* path, const char* mode);

int adafs_close(char* path);

int adafs_stat(char* path, struct stat* buf);

ssize_t adafs_write(char* path, char* buf, size_t size, off_t offset);

ssize_t adafs_read(char* path, char* buf, size_t size, off_t offset);


#endif //IFS_FS_OPERATIONS_HPP
