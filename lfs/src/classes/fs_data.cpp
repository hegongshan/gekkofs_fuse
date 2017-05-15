//
// Created by evie on 4/18/17.
//

#include "fs_data.h"

const std::unordered_map<std::string, std::string>& FsData::hashmap() const {
    return hashmap_;
}

void FsData::hashmap(const std::unordered_map<std::string, std::string>& hashmap_) {
    FsData::hashmap_ = hashmap_;
}

const std::hash<std::string>& FsData::hashf() const {
    return hashf_;
}

void FsData::hashf(const std::hash<std::string>& hashf_) {
    FsData::hashf_ = hashf_;
}

blksize_t FsData::blocksize() const {
    return blocksize_;
}

void FsData::blocksize(blksize_t blocksize_) {
    FsData::blocksize_ = blocksize_;
}

const std::shared_ptr<spdlog::logger>& FsData::spdlogger() const {
    return spdlogger_;
}

void FsData::spdlogger(const std::shared_ptr<spdlog::logger>& spdlogger_) {
    FsData::spdlogger_ = spdlogger_;
}

const std::string& FsData::rootdir() const {
    return rootdir_;
}

void FsData::rootdir(const std::string& rootdir_) {
    FsData::rootdir_ = rootdir_;
}

const std::string& FsData::inode_path() const {
    return inode_path_;
}

void FsData::inode_path(const std::string& inode_path_) {
    FsData::inode_path_ = inode_path_;
}

const std::string& FsData::dentry_path() const {
    return dentry_path_;
}

void FsData::dentry_path(const std::string& dentry_path_) {
    FsData::dentry_path_ = dentry_path_;
}

const std::string& FsData::chunk_path() const {
    return chunk_path_;
}

void FsData::chunk_path(const std::string& chunk_path_) {
    FsData::chunk_path_ = chunk_path_;
}

const std::string& FsData::mgmt_path() const {
    return mgmt_path_;
}

void FsData::mgmt_path(const std::string& mgmt_path_) {
    FsData::mgmt_path_ = mgmt_path_;
}

