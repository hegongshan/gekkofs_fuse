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
//    rocksdb::DB* rdb_;
    std::shared_ptr<rocksdb::DB> rdb_; //single rocksdb instance
    rocksdb::Options rdb_options_;
    std::string rdb_path_;

public:
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

//    rocksdb::DB* rdb();
//
//    void rdb(rocksdb::DB* rdb_);

    const std::shared_ptr<rocksdb::DB>& rdb() const;

    void rdb(const std::shared_ptr<rocksdb::DB>& rdb);

    const rocksdb::Options& rdb_options() const;

    void rdb_options(const rocksdb::Options& rdb_options);

    const std::string& rdb_path() const;

    void rdb_path(const std::string& rdb_path);

};


#endif //LFS_FS_DATA_H
