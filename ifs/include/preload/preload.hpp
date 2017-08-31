//
// Created by evie on 7/21/17.
//

#ifndef IOINTERCEPT_PRELOAD_HPP
#define IOINTERCEPT_PRELOAD_HPP

#include <sys/statfs.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>

#include "preload/open_file_map.hpp"

//
void* libc;
void* libc_close;
void* libc___close;
void* libc_puts;
void* libc_write;
void* libc_read;
void* libc_open;
void* libc_fopen;
//void* libc_fclose;


static OpenFileMap file_map{};

#define ld_puts puts
#define ld_write write
#define ld_read read
#define ld_open open
#define ld_fopen fopen
//#define ld_fclose fclose
#define ld_close close
#define ld___close __close

void init_preload(void) __attribute__((constructor));

void destroy_preload(void) __attribute__((destructor));

#endif //IOINTERCEPT_PRELOAD_HPP
