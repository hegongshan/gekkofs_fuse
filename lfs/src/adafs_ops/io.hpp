//
// Created by evie on 4/3/17.
//

#ifndef FS_IO_H
#define FS_IO_H

#include "../main.hpp"

int init_chunk_space(const fuse_ino_t inode);

int destroy_chunk_space(const fuse_ino_t inode);

int read_file(char* buf, size_t& read_size, const char* path, const size_t size, const off_t off);

int write_file(const fuse_ino_t inode, const char *buf, size_t &write_size, const size_t size, const off_t off,
               const bool append);


#endif //FS_IO_H
