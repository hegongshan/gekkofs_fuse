//
// Created by evie on 6/6/17.
//

#include "db_ops.hpp"

using namespace rocksdb;
using namespace std;

inline const bool db_get_mdata_helper(const string& key, string& val) {
    auto db = ADAFS_DATA->rdb();
    return db->Get(ReadOptions(), key, &val).ok();
}

template<>
unsigned long db_get_mdata<unsigned long>(const string& key) {
    string val;
    if (db_get_mdata_helper(key, val))
        return stoul(val);
    else
        return 0;
}

template<>
long db_get_mdata<long>(const string& key) {
    string val;
    if (db_get_mdata_helper(key, val))
        return stol(val);
    else
        return 0;
}

template<>
unsigned int db_get_mdata<unsigned int>(const string& key) {
    string val;
    if (db_get_mdata_helper(key, val))
        return static_cast<unsigned int>(stoul(val));
    else
        return 0;
}

bool db_delete_mdata(const string& key) {
    auto db = ADAFS_DATA->rdb();
    return db->Delete(ADAFS_DATA->rdb_write_options(), key).ok();
}

bool db_dentry_exists(const fuse_ino_t p_inode, const string& name, string& val) {
    auto db = ADAFS_DATA->rdb();
    auto key = db_build_dentry_key(p_inode, name);
    return db->Get(rocksdb::ReadOptions(), key, &val).ok();
}

bool db_mdata_exists(const fuse_ino_t inode) {
    auto db = ADAFS_DATA->rdb();
    string val_str;
    return db->Get(ReadOptions(),
                   db_build_mdata_key(inode, std::get<to_underlying(Md_fields::atime)>(md_field_map)),
                   &val_str).ok();
}

bool db_put_dentry(const string& key, const string& val) {
    auto db = ADAFS_DATA->rdb();
    return db->Put(ADAFS_DATA->rdb_write_options(), key, val).ok();
}

void db_get_dentries(vector<Dentry>& dentries, const fuse_ino_t dir_inode) {
    string key;
    string val;
    size_t pos;
    auto delim = "_"s;
    auto db = ADAFS_DATA->rdb();
    auto prefix = db_build_dentry_prefix(dir_inode);
    // Do RangeScan on parent inode
    auto dentry_iter = db->NewIterator(rocksdb::ReadOptions());
    for (dentry_iter->Seek(prefix);
         dentry_iter->Valid() && dentry_iter->key().starts_with(prefix); dentry_iter->Next()) {
        ADAFS_DATA->spdlogger()->trace("Dentry:");
        key = dentry_iter->key().ToString();
        val = dentry_iter->value().ToString();
        ADAFS_DATA->spdlogger()->trace("key '{}' value '{}'", key, val);

        // Retrieve filename from key
        key.erase(0, 2); // Erase prefix <d_>
        pos = key.find(delim); // Split <ParentInode_filename> by _
        key.erase(0, pos + 1); // Erase ParentInode + _
        Dentry dentry{key}; // key holds only filename

        // Retrieve inode and mode from val
        pos = val.find(delim); // Split <inode_mode> by _
        dentry.inode(static_cast<fuse_ino_t>(stoul(val.substr(0, pos)))); // Substring from 0 to pos holds inode
        val.erase(0, pos + 1); // Erase inode + delim
        dentry.mode(static_cast<mode_t>(stoul(val))); // val holds only mode
        // append dentry to dentries vector
        ADAFS_DATA->spdlogger()->trace("Formatted: name {} inode {} mode {:o}", dentry.name(), dentry.inode(),
                                       dentry.mode());
        dentries.push_back(dentry);
    }
}

pair<int, fuse_ino_t> db_delete_dentry_get_inode(const fuse_ino_t p_inode, const string& name) {
    auto key = db_build_dentry_key(p_inode, name);
    auto db = ADAFS_DATA->rdb();
    string val;
    db->Get(ReadOptions(), key, &val);
    auto pos = val.find("_");

    return make_pair(db->Delete(ADAFS_DATA->rdb_write_options(), key).ok() ? 0 : EIO,
                     static_cast<fuse_ino_t>(stoul(val.substr(0, pos))));
}

/**
 * Returns true if no dentries can be found for the prefix <d_ParentInode>
 * @param inode
 * @return bool
 */
bool db_is_dir_empty(const fuse_ino_t inode) {
    auto dir_empty = true;
    auto db = ADAFS_DATA->rdb();
    auto prefix = db_build_dentry_prefix(inode);
    auto dentry_iter = db->NewIterator(rocksdb::ReadOptions());
    for (dentry_iter->Seek(prefix);
         dentry_iter->Valid() && dentry_iter->key().starts_with(prefix); dentry_iter->Next()) {
        dir_empty = false;
        break;
    }
    return dir_empty;
}



