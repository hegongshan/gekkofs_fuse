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


ssize_t ld_write(int fd, const void *buf, size_t count) {
    printf("write:chars#:%lu, buf %s\n", count, (char*)buf);
    printf("\tfd:%d\n", fd);
    printf("\tPath in write %s\n", file_map.get(fd)->path());
    return (reinterpret_cast<decltype(&write)>(libc_write))(fd, buf, count);
}

ssize_t ld_read(int fd, void *buf, size_t count) {
    return (reinterpret_cast<decltype(&read)>(libc_read))(fd, buf, count);
}


int ld_puts(const char* str) {
    printf("puts:chars#:%lu\n", strlen(str));
    return (reinterpret_cast<decltype(&puts)>(libc_puts))(str);
}

int ld_open(const char *path, int flags, ...){
    printf("opening up the path: %s\n", path);
    if(flags & O_CREAT){
        va_list vl;
        va_start(vl,flags);
        mode_t mode = va_arg(vl,int);
        va_end(vl);
        int ret = (reinterpret_cast<decltype(&open)>(libc_open))(path, flags, mode);
        printf("\tGot fd:%d\n", ret);
        file_map.add(path, ret);
        return ret;
    }
    int ret = (reinterpret_cast<decltype(&open)>(libc_open))(path, flags);
    file_map.add(path, ret);
    printf("\tGot fd:%d\n", ret);
    return ret;
}

FILE* ld_fopen(const char *path, const char *mode){
    printf("FILE opening up the path: %s\n", path);
    return (reinterpret_cast<decltype(&fopen)>(libc_fopen))(path, mode);
}

int ld_close(int fd) {
    printf("closing fd: %d\n", fd);
    file_map.remove(fd);
    return (reinterpret_cast<decltype(&close)>(libc_close))(fd);

}

int ld___close(int fd) {
    printf("___closing fd: %d\n", fd);
    return ld_close(fd);
}

void init_preload(void) {
    libc = dlopen("libc.so.6", RTLD_LAZY);
    libc_close = dlsym(libc, "close");
    libc___close = dlsym(libc, "__close");
    libc_puts = dlsym(libc, "puts");
    libc_write = dlsym(libc, "write");
    libc_read = dlsym(libc, "read");
    libc_open = dlsym(libc, "open");
    libc_fopen = dlsym(libc, "fopen");
    printf("HELLLO\n");
}

void destroy_preload(void) {

}