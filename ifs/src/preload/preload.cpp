//
// Created by evie on 7/21/17.
//

//#define _GNU_SOURCE
#include "preload/preload.hpp"
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

//#include <dlfcn.h>
//#include <sys/types.h>
//#include <sys/uio.h>
//#include <stdio.h>
#include <stdarg.h>
//#include <stdlib.h>
//#include <inttypes.h>
//#include <string.h>
//#include <assert.h>
//#include <errno.h>
//#include <utime.h>
//#include <sys/statvfs.h>
//#include <fcntl.h>
//#include <sys/stat.h>
//#include <dirent.h>
//#include <sys/param.h>
//#include <sys/mount.h>
//#include <sys/time.h>
#include <unistd.h>
//#include <dirent.h>
//#include <sys/xattr.h>
//#include <string>
//#include <iostream>


int ld_open(const char *path, int flags, ...){
    printf("opening up the path: %s\n", path);
    mode_t mode;
    if(flags & O_CREAT){
        va_list vl;
        va_start(vl,flags);
        mode = va_arg(vl, int);
        va_end(vl);
    }
    if (is_fs_path(path)) {
        auto fd = file_map.add(path);
        // TODO call daemon and return if successful return the above fd. if unsuccessful delete fd remove file from map
    }
    return (reinterpret_cast<decltype(&open)>(libc_open))(path, flags, mode);
}

int ld_open64(__const char *path, int flags, ...) {
    mode_t mode;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return ld_open(path, flags | O_LARGEFILE, mode);
}

FILE* ld_fopen(const char *path, const char *mode){
    printf("FILE opening up the path: %s\n", path);
    return (reinterpret_cast<decltype(&fopen)>(libc_fopen))(path, mode);
}

int ld_creat(const char *pathname, mode_t mode) {
    return ld_open(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

int ld_close(int fd) {
    if (file_map.exist(fd)) {
        // TODO call daemon and return
        file_map.remove(fd);
    } else
        printf("closing fd: %d\n", fd);
    return (reinterpret_cast<decltype(&close)>(libc_close))(fd);
}

int ld___close(int fd) {
    printf("___closing fd: %d\n", fd);
    return ld_close(fd);
}

// XXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXX Continue here
int ld_stat(const char *pathname, struct stat *buf) {
    printf("stat called with path %s\n", pathname);
    return (reinterpret_cast<decltype(&stat)>(libc_stat))(pathname, buf);
}

int ld_fstat(int fd, struct stat *buf) {
    printf("stat called with fd %d\n", fd);
    return (reinterpret_cast<decltype(&fstat)>(libc_fstat))(fd, buf);
}

int ld_puts(const char* str) {
    printf("puts:chars#:%lu\n", strlen(str));
    return (reinterpret_cast<decltype(&puts)>(libc_puts))(str);
}

ssize_t ld_write(int fd, const void *buf, size_t count) {
    printf("write:chars#:%lu, buf %s\n", count, (char*)buf);
    printf("\tfd:%d\n", fd);
    printf("\tPath in write %s\n", file_map.get(fd)->path());
    return (reinterpret_cast<decltype(&write)>(libc_write))(fd, buf, count);
}

ssize_t ld_read(int fd, void *buf, size_t count) {
    return (reinterpret_cast<decltype(&read)>(libc_read))(fd, buf, count);
}

ssize_t ld_pread(int fd, void *buf, size_t count, off_t offset) {
    return (reinterpret_cast<decltype(&pread)>(libc_pread))(fd, buf, count, offset);
}

ssize_t ld_pread64(int fd, void* buf, size_t nbyte, __off64_t offset) {
    return (reinterpret_cast<decltype(&pread64)>(libc_pread64))(fd, buf, nbyte, offset);
}

off_t ld_lseek(int fd, off_t offset, int whence) __THROW {
    return (reinterpret_cast<decltype(&lseek)>(libc_lseek))(fd, offset, whence);
}

off_t ld_lseek64(int fd, off_t offset, int whence) __THROW {
    return ld_lseek(fd, offset, whence);
}

int ld_truncate(const char *path, off_t length) __THROW {
    return (reinterpret_cast<decltype(&truncate)>(libc_truncate))(path, length);
}

int ld_ftruncate(int fd, off_t length) __THROW {
    return (reinterpret_cast<decltype(&ftruncate)>(libc_ftruncate))(fd, length);
}

int ld_dup(int oldfd) __THROW {
    return (reinterpret_cast<decltype(&dup)>(libc_dup))(oldfd);
}

int ld_dup2(int oldfd, int newfd) __THROW {
    return (reinterpret_cast<decltype(&dup2)>(libc_dup2))(oldfd, newfd);
}

void init_preload(void) {
    libc = dlopen("libc.so.6", RTLD_LAZY);
    libc_open = dlsym(libc, "open");
    libc_fopen = dlsym(libc, "fopen");

    libc_close = dlsym(libc, "close");
//    libc___close = dlsym(libc, "__close");

    libc_stat = dlsym(libc, "stat");
    libc_fstat = dlsym(libc, "fstat");

    libc_puts = dlsym(libc, "puts");

    libc_write = dlsym(libc, "write");
    libc_read = dlsym(libc, "read");
    libc_pread = dlsym(libc, "pread");
    libc_pread64 = dlsym(libc, "pread64");

    libc_lseek = dlsym(libc, "lseek");

    libc_truncate = dlsym(libc, "truncate");
    libc_ftruncate = dlsym(libc, "ftruncate");

    printf("HELLLO\n");
}

void destroy_preload(void) {

}