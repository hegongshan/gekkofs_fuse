//
// Created by evie on 6/6/17.
//

#include "db_ops.hpp"

using namespace rocksdb;
using namespace std;

inline const string db_get_mdata_helper(const char* key) {
    auto db = ADAFS_DATA->rdb().get();
    string val_str;
    db->Get(ReadOptions(), key, &val_str);
    return val_str;
}

template<>
unsigned long db_get_mdata<unsigned long>(const char* key) {
    return stoul(db_get_mdata_helper(key));
}

template<>
long db_get_mdata<long>(const char* key) {
    return stol(db_get_mdata_helper(key));
}

template<>
unsigned int db_get_mdata<unsigned int>(const char* key) {
    return static_cast<unsigned int>(stoul(db_get_mdata_helper(key)));
}

bool db_dentry_exists(const char* key) {
//    auto db = ADAFS_DATA->rdb().get();
    //TODO
    return true;
}

bool db_mdata_exists(const char* key) {
    auto db = ADAFS_DATA->rdb().get();
    string val_str;
    return db->Get(ReadOptions(), key, &val_str).ok();
}
