//
// Created by evie on 6/8/17.
//

#include "db_util.hpp"

using namespace std;

bool init_rocksdb() {
    rocksdb::DB* db;
    ADAFS_DATA->rdb_path(ADAFS_DATA->rootdir() + "/meta/rocksdb"s);
    rocksdb::Options options;
    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    // create the DB if it's not already present
    options.create_if_missing = true;


    optimize_rocksdb(options);

    ADAFS_DATA->rdb_options(options);
//    rocksdb::OptimisticTransactionDB* txn_db;
//    rocksdb::OptimisticTransactionOptions txn_options{};
//    ADAFS_DATA->txn_rdb_options(txn_options);
    ADAFS_DATA->spdlogger()->info("RocksDB options set. About to connect...");
    // open DB
//    auto s = rocksdb::OptimisticTransactionDB::Open(ADAFS_DATA->rdb_options(), ADAFS_DATA->rdb_path(), &txn_db);
    auto s = rocksdb::DB::Open(ADAFS_DATA->rdb_options(), ADAFS_DATA->rdb_path(), &db);

    if (s.ok()) {
//        db = txn_db->GetBaseDB(); // db connection for db operations without transactions
        shared_ptr<rocksdb::DB> s_db(db);
        ADAFS_DATA->rdb(s_db);
//        shared_ptr<rocksdb::OptimisticTransactionDB> s_txn_db(txn_db);
//        ADAFS_DATA->txn_rdb(s_txn_db);
        ADAFS_DATA->spdlogger()->info("RocksDB connection established.");
        return true;
    } else {
        ADAFS_DATA->spdlogger()->info("[ERROR] RocksDB connection FAILURE. Exiting...");
        return false;
    }
}

void optimize_rocksdb(rocksdb::Options& options) {

    //    rocksdb::BlockBasedTableOptions block_options{};
//    block_options.block_size = 16384 * 2;
//    options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(block_options));
    // experimental settings
//    options.write_buffer_size = 512;
//    options.max_write_buffer_number = 16;
//    options.min_write_buffer_number_to_merge = 4;
    // These 4 below have the most impact
    options.max_bytes_for_level_base = 2048;
    options.max_bytes_for_level_multiplier = 10;
    options.target_file_size_base = 256;
    options.target_file_size_multiplier = 1;
    //
    options.max_background_flushes = 1;
    options.max_background_compactions = 48;
    options.level0_file_num_compaction_trigger = 1;
    options.level0_slowdown_writes_trigger = 48;
    options.level0_stop_writes_trigger = 56;
//    options.arena_block_size = 1024 * 8;
//    options.compression = rocksdb::kNoCompression; // doesnt do anything

    // Disable Write-Ahead Logging if configured
    rocksdb::WriteOptions write_options{};
#ifndef RDB_WOL
    write_options.disableWAL = true;
#endif
    ADAFS_DATA->rdb_write_options(write_options);
}




/**
 * Build dentry key of form <d_ParentInode_filename>
 * @param inode
 * @param name
 * @return
 */
string db_build_dentry_key(const fuse_ino_t inode, const string& name) {
    return ("d_"s + fmt::FormatInt(inode).str() + "_" + name);
}

/**
 * Build dentry prefix of form <d_ParentInode>
 * @param inode
 * @param name
 * @return
 */
string db_build_dentry_prefix(const fuse_ino_t inode) {
    return ("d_"s + fmt::FormatInt(inode).str() + "_"s);
}

string db_build_dentry_value(const fuse_ino_t inode, const mode_t mode) {
    return (fmt::FormatInt(inode).str() + "_"s + fmt::FormatInt(mode).str());
}

/**
 * Build mdata key of form <inode_fieldname>
 * @param inode
 * @param name
 * @return
 */
string db_build_mdata_key(const fuse_ino_t inode, const string& field) {
    return (fmt::FormatInt(inode).str() + field);
}

string db_build_mdata_key(const string& inode, const string& field) {
    return (inode + field);
}

vector<string> db_build_all_mdata_keys(const fuse_ino_t inode) {
    auto inode_key = fmt::FormatInt(inode).str();
    vector<string> mdata_keys{};
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::atime)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::mtime)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::ctime)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::uid)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::gid)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::mode)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::inode_no)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::link_count)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::size)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::blocks)>(md_field_map));
    return mdata_keys;
}


