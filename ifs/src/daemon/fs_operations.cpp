//
// Created by evie on 9/4/17.
//

#include "daemon/fs_operations.hpp"

int adafs_open(char* path, int flags, mode_t mode) {
    if (flags & O_CREAT) {
        // do file create
    } else {
        // do nothing.
    }
    // build file descriptor
    return 0;
}

FILE* adafs_fopen(char* path, const char* mode) {
    return nullptr;
}

int adafs_close(char* path) {
    return 0;
}

int adafs_stat(char* path, struct stat* buf) {
    return 0;
}

int adafs_fstat(char* path, struct stat* buf) {
    return 0;
}

ssize_t adafs_write(char* path, void* buf, size_t count) {
    return 0;
}

ssize_t adafs_read(char* path, void* buf, size_t count) {
    return 0;
}

ssize_t adafs_pread(char* path, void* buf, size_t count, off_t offset) {
    return 0;
}