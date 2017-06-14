//
// Created by evie on 6/8/17.
//

#ifndef LFS_DB_TXN_OPS_HPP
#define LFS_DB_TXN_OPS_HPP

#include "../main.hpp"
#include "util.hpp"

template<typename T>
bool dbtxn_put_mdata(const std::string& key, const T val, rocksdb::Transaction& txn) {
    auto val_str = fmt::FormatInt(val).c_str();
    return txn.Put(key, val_str).ok();
}

template<typename T>
T dbtxn_get_mdata(const std::string& key, rocksdb::Transaction& txn);

bool dbtxn_delete_mdata(const std::string& key, rocksdb::Transaction& txn);

bool
dbtxn_dentry_exists(const fuse_ino_t p_inode, const std::string& name, std::string& val, rocksdb::Transaction& txn);

bool dbtxn_mdata_exists(const fuse_ino_t inode, rocksdb::Transaction& txn);

bool dbtxn_put_dentry(const std::string& key, const std::string& val, rocksdb::Transaction& txn);

std::pair<bool, fuse_ino_t>
dbtxn_delete_dentry_get_inode(const fuse_ino_t p_inode, const std::string& name, rocksdb::Transaction& txn);

#endif //LFS_DB_TXN_OPS_HPP
