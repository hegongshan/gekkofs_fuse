//
// Created by evie on 6/8/17.
//

#include "db_txn_ops.hpp"

using namespace rocksdb;
using namespace std;

inline const string dbtxn_get_mdata_helper(const string& key, rocksdb::Transaction& txn) {
    string val_str;
    txn.Get(ReadOptions(), key, &val_str);
    return val_str;
}

template<>
unsigned long dbtxn_get_mdata<unsigned long>(const string& key, rocksdb::Transaction& txn) {
    return stoul(dbtxn_get_mdata_helper(key, txn));
}

template<>
long dbtxn_get_mdata<long>(const string& key, rocksdb::Transaction& txn) {
    return stol(dbtxn_get_mdata_helper(key, txn));
}

template<>
unsigned int dbtxn_get_mdata<unsigned int>(const string& key, rocksdb::Transaction& txn) {
    return static_cast<unsigned int>(stoul(dbtxn_get_mdata_helper(key, txn)));
}

bool dbtxn_delete_mdata(const string& key, rocksdb::Transaction& txn) {
    return txn.Delete(key).ok();
}

bool dbtxn_dentry_exists(const fuse_ino_t p_inode, const string& name, string& val, rocksdb::Transaction& txn) {
    auto key = db_build_dentry_key(p_inode, name);
    return txn.Get(ReadOptions(), key, &val).ok();
}

bool dbtxn_mdata_exists(const fuse_ino_t inode, rocksdb::Transaction& txn) {
    string val_str;
    return txn.Get(ReadOptions(),
                   db_build_mdata_key(inode, std::get<to_underlying(Md_fields::atime)>(md_field_map)),
                   &val_str).ok();
}

bool dbtxn_put_dentry(const string& key, const string& val, rocksdb::Transaction& txn) {
    return txn.Put(key, val).ok();
}


pair<bool, fuse_ino_t>
dbtxn_delete_dentry_get_inode(const fuse_ino_t p_inode, const string& name, rocksdb::Transaction& txn) {
    auto key = db_build_dentry_key(p_inode, name);
    string val;
    txn.Get(ReadOptions(), key, &val);
    auto pos = val.find("_");

    return make_pair(txn.Delete(key).ok() ? 0 : 1, static_cast<fuse_ino_t>(stoul(val.substr(0, pos))));
}
