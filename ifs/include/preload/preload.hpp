//
// Created by evie on 7/21/17.
//

#ifndef IOINTERCEPT_PRELOAD_HPP
#define IOINTERCEPT_PRELOAD_HPP

#include <sys/statfs.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>

#include "../../configure.hpp"

extern "C" {
#include <abt.h>
#include <abt-snoozer.h>
#include <mercury_types.h>
#include <mercury_proc_string.h>
#include <margo.h>
}

#include <preload/open_file_map.hpp>
#include <preload/preload_util.hpp>


void* libc;

void* libc_open;
//void* libc_open64; //unused
void* libc_fopen;

//void* libc_creat; //unused

void* libc_close;
//void* libc___close; //unused

void* libc_stat;
void* libc_fstat;

void* libc_puts;

void* libc_write;
void* libc_read;
void* libc_pread;
void* libc_pread64;

void* libc_lseek;
//void* libc_lseek64; //unused

void* libc_truncate;
void* libc_ftruncate;

void* libc_dup;
void* libc_dup2;


static OpenFileMap file_map{};

#define ld_open open
#define ld_open64 open64
#define ld_fopen fopen

#define ld_creat creat

#define ld_close close
#define ld___close __close

#define ld_stat stat
#define ld_fstat fstat

#define ld_puts puts

#define ld_write write
#define ld_read read
#define ld_pread pread
#define ld_pread64 pread64

#define ld_lseek lseek
#define ld_lseek64 lseek64

#define ld_truncate truncate
#define ld_ftruncate ftruncate

#define ld_dup dup
#define ld_dup2 dup2

bool is_lib_initialized = false;

FILE* debug_fd;

#define DAEMON_DEBUG(fd, fmt, ...) \
            do { if (LOG_DAEMON_DEBUG) fprintf(fd, "[" __DATE__ ":" __TIME__ "] " fmt, ##__VA_ARGS__); } while (0)
#define DAEMON_DEBUG0(fd, fmt) \
            do { if (LOG_DAEMON_DEBUG) fprintf(fd, "[" __DATE__ ":" __TIME__ "] " fmt); } while (0)


bool init_argobots();

void register_client_ipcs();

bool init_ipc_client();


void init_preload(void) __attribute__((constructor));

void destroy_preload(void) __attribute__((destructor));

#endif //IOINTERCEPT_PRELOAD_HPP
