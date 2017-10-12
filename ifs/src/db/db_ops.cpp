//
// Created by evie on 9/6/17.
//

#include <db/db_ops.hpp>

using namespace rocksdb;
using namespace std;

bool db_get_metadentry(const std::string& key, std::string& val) {
    auto db = ADAFS_DATA->rdb();
    auto err = db->Get(ReadOptions(), key, &val).ok();
    // TODO check what happens if nothing could have been found. Will val be NULL, nullptr, ""?
    // It matters because the client RPC is checking for an empty string to see if get_attr was successful or not
    return err;
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

/**
 * Updates a metadentry atomically and also allows to change keys
 * @param old_key
 * @param new_key
 * @param val
 * @return
 */
bool db_update_metadentry(const std::string& old_key, const std::string& new_key, const std::string& val) {
    auto db = ADAFS_DATA->rdb();
    rocksdb::WriteBatch batch;
    batch.Delete(old_key);
    batch.Put(new_key, val);
    return db->Write(ADAFS_DATA->rdb_write_options(), &batch).ok();
}