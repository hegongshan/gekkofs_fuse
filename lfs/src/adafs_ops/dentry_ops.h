//
// Created by evie on 3/17/17.
//

#ifndef FS_DENTRY_OPS_H
#define FS_DENTRY_OPS_H

#include "../main.h"
#include "../classes/dentry.h"


bool init_dentry_dir(const fuse_ino_t inode);

bool destroy_dentry_dir(const fuse_ino_t inode);

bool verify_dentry(const fuse_ino_t inode);

int read_dentries(const fuse_ino_t p_inode, const fuse_ino_t inode);

int get_dentries(std::vector<Dentry>& dentries, const fuse_ino_t dir_inode);

std::pair<int, fuse_ino_t> do_lookup(fuse_req_t& req, const fuse_ino_t p_inode, const std::string& name);

int create_dentry(const fuse_ino_t p_inode, const fuse_ino_t inode, const std::string& name, mode_t mode);

int remove_dentry(const fuse_ino_t p_inode, const fuse_ino_t inode);

bool is_dir_empty(const fuse_ino_t inode);

#endif //FS_DENTRY_OPS_H
