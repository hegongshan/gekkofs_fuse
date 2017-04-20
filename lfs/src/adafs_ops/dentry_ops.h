//
// Created by evie on 3/17/17.
//

#ifndef FS_DENTRY_OPS_H
#define FS_DENTRY_OPS_H

#include "../main.h"

bool init_dentry_dir(const uint64_t inode);

bool destroy_dentry_dir(const uint64_t inode);

bool verify_dentry(const uint64_t inode);

int read_dentries(const uint64_t p_inode, const unsigned long inode);

int create_dentry(const unsigned long p_inode, const uint64_t inode);

int remove_dentry(const unsigned long p_inode, const uint64_t inode);

bool is_dir_empty(const uint64_t inode);

#endif //FS_DENTRY_OPS_H
