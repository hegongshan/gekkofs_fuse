//
// Created by evie on 6/6/17.
//

#ifndef LFS_DB_OPS_HPP
#define LFS_DB_OPS_HPP

#include "../main.hpp"

template<typename T>
bool db_put_mdata(const char* key, const T val) {
    auto db = ADAFS_DATA->rdb().get();
    auto val_str = fmt::FormatInt(val).c_str();
    return db->Put(rocksdb::WriteOptions(), key, val_str).ok();
}

template<typename T>
T db_get_mdata(const char* key);

bool db_mdata_exists(const char* key);

#endif //LFS_DB_OPS_HPP
