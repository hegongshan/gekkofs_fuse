//
// Created by evie on 9/6/17.
//

#include "db/db_ops.hpp"

using namespace rocksdb;
using namespace std;

std::string db_get_metadentry(const std::string& key) {
    auto db = ADAFS_DATA->rdb();
    string val;
    db->Get(ReadOptions(), key, &val).ok();

    return val;
}

bool db_put_metadentry(const std::string& key, const std::string& val) {
    auto db = ADAFS_DATA->rdb();
    return db->Put(ADAFS_DATA->rdb_write_options(), key, val).ok();
}

bool db_delete_metadentry(const std::string& key) {
    auto db = ADAFS_DATA->rdb();
    return db->Delete(ADAFS_DATA->rdb_write_options(), key).ok();
}

bool db_metadentry_exists(const std::string& key) {
    auto db = ADAFS_DATA->rdb();
    string val_str;
    return db->Get(ReadOptions(), key, &val_str).ok();
}

bool db_is_dir_entry(const std::string& dir_path) {
    // TODO
    return true;
}