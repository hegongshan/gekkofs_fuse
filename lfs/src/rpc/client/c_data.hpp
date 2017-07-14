//
// Created by evie on 7/13/17.
//

#ifndef LFS_C_DATA_HPP
#define LFS_C_DATA_HPP

#include "../../main.hpp"

int rpc_send_read(const size_t recipient, const fuse_ino_t inode, const size_t in_size, const off_t in_offset,
                  char* tar_buf);

#endif //LFS_C_DATA_HPP
