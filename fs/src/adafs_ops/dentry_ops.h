//
// Created by evie on 3/17/17.
//

#ifndef FS_DENTRY_OPS_H
#define FS_DENTRY_OPS_H

#include "../main.h"

bool init_dentry(const unsigned long& hash);

bool verify_dentry(const bfs::path& path);

int read_dentries(std::vector<std::string>& dir, const unsigned long hash);

int create_dentry(const unsigned long parent_dir_hash, const std::string& fname);

#endif //FS_DENTRY_OPS_H
