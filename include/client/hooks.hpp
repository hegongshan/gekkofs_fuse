/*
  Copyright 2018-2019, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2019, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#ifndef IFS_HOOKS_HPP
#define IFS_HOOKS_HPP

#include <fcntl.h>


int hook_openat(int dirfd, const char *cpath, int flags, mode_t mode);
int hook_close(int fd);
int hook_stat(const char* path, struct stat* buf);
int hook_lstat(const char* path, struct stat* buf);
int hook_fstat(unsigned int, struct stat* buf);
int hook_read(int fd, void* buf, size_t count);
int hook_write(int fd, void* buf, size_t count);
int hook_unlink(const char* path);
int hook_access(const char* path, int mask);
int hook_lseek(unsigned int fd, off_t offset, unsigned int whence);


#endif
