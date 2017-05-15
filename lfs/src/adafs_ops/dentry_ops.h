//
// Created by evie on 3/17/17.
//

#ifndef FS_DENTRY_OPS_H
#define FS_DENTRY_OPS_H

#include "../main.h"
#include "../classes/dentry.h"


bool init_dentry_dir(const uint64_t inode);

bool destroy_dentry_dir(const uint64_t inode);

bool verify_dentry(const uint64_t inode);

int read_dentries(const uint64_t p_inode, const unsigned long inode);

int get_dentries(std::vector<Dentry>& dentries, const uint64_t dir_inode);

std::pair<int, uint64_t> do_lookup(fuse_req_t& req, const uint64_t p_inode, const std::string& name);

int create_dentry(const uint64_t p_inode, const uint64_t inode, const std::string& name, mode_t mode);

int remove_dentry(const unsigned long p_inode, const uint64_t inode);

bool is_dir_empty(const uint64_t inode);

#endif //FS_DENTRY_OPS_H
