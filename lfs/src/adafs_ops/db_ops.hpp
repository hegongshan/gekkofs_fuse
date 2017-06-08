//
// Created by evie on 6/6/17.
//

#ifndef LFS_DB_OPS_HPP
#define LFS_DB_OPS_HPP

#include "../main.hpp"
#include "../classes/dentry.h"

template<typename T>
bool db_put_mdata(const std::string& key, const T val) {
    auto db = ADAFS_DATA->rdb().get();
    auto val_str = fmt::FormatInt(val).c_str();
    return db->Put(rocksdb::WriteOptions(), key, val_str).ok();
}

template<typename T>
T db_get_mdata(const std::string& key);

bool db_dentry_exists(const fuse_ino_t p_inode, const std::string& name, std::string& val);

bool db_mdata_exists(const fuse_ino_t inode);

bool db_put_dentry(const std::string& key, const std::string& val);

void db_get_dentries(std::vector<Dentry>& dentries, const fuse_ino_t dir_inode);

#endif //LFS_DB_OPS_HPP
