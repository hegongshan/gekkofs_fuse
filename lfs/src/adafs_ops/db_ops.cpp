//
// Created by evie on 6/6/17.
//

#include "db_ops.hpp"

using namespace rocksdb;
using namespace std;

bool db_put(const string key, const time_t val) {
    auto db = ADAFS_DATA->rdb().get();
    return db->Put(WriteOptions(), key, fmt::FormatInt(val).c_str()).ok();
}

time_t db_get(const string key) {
    auto db = ADAFS_DATA->rdb().get();
    string val_s;
    db->Get(ReadOptions(), key, &val_s);
    return static_cast<time_t>(stol(val_s));
}