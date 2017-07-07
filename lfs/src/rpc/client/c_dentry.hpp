//
// Created by evie on 7/7/17.
//

#ifndef LFS_C_DENTRY_HPP
#define LFS_C_DENTRY_HPP

#include "../../main.hpp"

int rpc_send_lookup(const size_t recipient, const fuse_ino_t parent, const char* name, fuse_ino_t& inode);


#endif //LFS_C_DENTRY_HPP
