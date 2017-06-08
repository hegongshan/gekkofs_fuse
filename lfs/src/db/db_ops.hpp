//
// Created by evie on 6/6/17.
//

#ifndef LFS_DB_OPS_HPP
#define LFS_DB_OPS_HPP

#include "../main.hpp"
#include "../classes/dentry.h"
#include "util.hpp"

template<typename T>
bool db_put_mdata(const std::string& key, const T val) {
    auto val_str = fmt::FormatInt(val).c_str();
    auto db = ADAFS_DATA->rdb();
    return db->Put(rocksdb::WriteOptions(), key, val_str).ok();
}

template<typename T>
T db_get_mdata(const std::string& key);

bool db_delete_mdata(const std::string& key);

bool db_dentry_exists(const fuse_ino_t p_inode, const std::string& name, std::string& val);

bool db_mdata_exists(const fuse_ino_t inode);

bool db_put_dentry(const std::string& key, const std::string& val);

void db_get_dentries(std::vector<Dentry>& dentries, const fuse_ino_t dir_inode);

std::pair<bool, fuse_ino_t> db_delete_dentry_get_inode(const fuse_ino_t p_inode, const std::string& name);

bool db_is_dir_empty(const fuse_ino_t inode);

#endif //LFS_DB_OPS_HPP
