//
// Created by evie on 4/3/17.
//

#ifndef FS_IO_H
#define FS_IO_H

#include "../main.hpp"

int init_chunk_space(const fuse_ino_t inode);

int destroy_chunk_space(const fuse_ino_t inode);

#endif //FS_IO_H
