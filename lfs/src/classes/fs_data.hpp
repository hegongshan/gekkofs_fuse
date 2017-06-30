//
// Created by evie on 4/18/17.
//

#ifndef LFS_FS_DATA_H
#define LFS_FS_DATA_H

#include "../main.hpp"

class FsData {

private:
    FsData() {}

    // Caching
    std::unordered_map<std::string, std::string> hashmap_;
    std::hash<std::string> hashf_;

    // inodes
    fuse_ino_t inode_count_;

    // Later the blocksize will likely be coupled to the chunks to allow individually big chunk sizes.
    blksize_t blocksize_;

    //logger
    std::shared_ptr<spdlog::logger> spdlogger_;

    // paths
    std::string rootdir_;
    std::string inode_path_;
    std::string dentry_path_;
    std::string chunk_path_;
    std::string mgmt_path_;

    // rocksdb
    std::shared_ptr<rocksdb::DB> rdb_;
    std::shared_ptr<rocksdb::DB> rdb_crt_; // additional db instance (currently not used)
    std::shared_ptr<rocksdb::OptimisticTransactionDB> txn_rdb_;
    rocksdb::Options rdb_options_;
    rocksdb::OptimisticTransactionOptions txn_rdb_options_; // needed for snapshots
    rocksdb::WriteOptions rdb_write_options_;
    std::string rdb_path_;

public:

    // mutex has a deleted method to assign an existing mutex. As such it cannot use getter or setters
    std::mutex inode_mutex;

    std::mutex m;
    std::condition_variable cv;


    static FsData* getInstance() {
        static FsData instance;
        return &instance;
    }

    FsData(FsData const&) = delete;

    void operator=(FsData const&) = delete;

    // getter/setter
    const std::unordered_map<std::string, std::string>& hashmap() const;

    void hashmap(const std::unordered_map<std::string, std::string>& hashmap_);

    const std::hash<std::string>& hashf() const;

    void hashf(const std::hash<std::string>& hashf_);

    fuse_ino_t inode_count() const;

    void inode_count(fuse_ino_t inode_count);

    blksize_t blocksize() const;

    void blocksize(blksize_t blocksize_);

    const std::shared_ptr<spdlog::logger>& spdlogger() const;

    void spdlogger(const std::shared_ptr<spdlog::logger>& spdlogger_);

    const std::string& rootdir() const;

    void rootdir(const std::string& rootdir_);

    const std::string& inode_path() const;

    void inode_path(const std::string& inode_path_);

    const std::string& dentry_path() const;

    void dentry_path(const std::string& dentry_path_);

    const std::string& chunk_path() const;

    void chunk_path(const std::string& chunk_path_);

    const std::string& mgmt_path() const;

    void mgmt_path(const std::string& mgmt_path_);

    const std::shared_ptr<rocksdb::DB>& rdb() const;

    void rdb(const std::shared_ptr<rocksdb::DB>& rdb);

    const rocksdb::Options& rdb_options() const;

    void rdb_options(const rocksdb::Options& rdb_options);

    const std::string& rdb_path() const;

    void rdb_path(const std::string& rdb_path);

    const std::shared_ptr<rocksdb::OptimisticTransactionDB>& txn_rdb() const;

    void txn_rdb(const std::shared_ptr<rocksdb::OptimisticTransactionDB>& tx_rdb);

    const std::shared_ptr<rocksdb::DB>& rdb_crt() const;

    void rdb_crt(const std::shared_ptr<rocksdb::DB>& rdb_crt);

    const rocksdb::OptimisticTransactionOptions& txn_rdb_options() const;

    void txn_rdb_options(const rocksdb::OptimisticTransactionOptions& tx_rdb_options);

    const rocksdb::WriteOptions& rdb_write_options() const;

    void rdb_write_options(const rocksdb::WriteOptions& rdb_write_options);

    // Utility member functions

    fuse_ino_t raise_inode_count(fuse_ino_t count);
};


#endif //LFS_FS_DATA_H
