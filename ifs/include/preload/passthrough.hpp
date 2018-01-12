//
// Created by evie on 11/15/17.
//

#ifndef IFS_PASSTHROUGH_HPP
#define IFS_PASSTHROUGH_HPP

#include <preload/preload.hpp>

// function pointer for preloading
extern void* libc;

extern void* libc_open;
//extern void* libc_open64; //unused
extern void* libc_fopen; // XXX Does not work with streaming pointers. If used will block forever
extern void* libc_fopen64; // XXX Does not work with streaming pointers. If used will block forever

//extern void* libc_creat; //unused
//extern void* libc_creat64; //unused
extern void* libc_mkdir;
extern void* libc_mkdirat;
extern void* libc_unlink;
extern void* libc_rmdir;

extern void* libc_close;
//extern void* libc___close; //unused

extern void* libc_stat;
extern void* libc_fstat;
extern void* libc___xstat;
extern void* libc___xstat64;
extern void* libc___fxstat;
extern void* libc___fxstat64;
extern void* libc___lxstat;
extern void* libc___lxstat64;

extern void* libc_access;

extern void* libc_puts;

extern void* libc_write;
extern void* libc_pwrite;
extern void* libc_pwrite64;
extern void* libc_read;
extern void* libc_pread;
extern void* libc_pread64;

extern void* libc_lseek;
//extern void* libc_lseek64; //unused

extern void* libc_truncate;
extern void* libc_ftruncate;

extern void* libc_dup;
extern void* libc_dup2;

void init_passthrough_if_needed();

#endif //IFS_PASSTHROUGH_HPP
