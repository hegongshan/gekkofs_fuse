//
// Created by evie on 4/18/17.
//

#ifndef LFS_FS_DATA_H
#define LFS_FS_DATA_H

#include "../main.h"

class FsData {

private:
    FsData() {}

    // Caching
    std::unordered_map<std::string, std::string> hashmap_;
    std::hash<std::string> hashf_;

    // Later the blocksize will likely be coupled to the chunks to allow individually big chunk sizes.
    int32_t blocksize_;

    //logger
    std::shared_ptr<spdlog::logger> spdlogger_;

    // paths
    std::string rootdir_;
    std::string inode_path_;
    std::string dentry_path_;
    std::string chunk_path_;
    std::string mgmt_path_;


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

    int32_t blocksize() const;

    void blocksize(int32_t blocksize_);

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

};


#endif //LFS_FS_DATA_H
