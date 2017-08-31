//
// Created by evie on 6/8/17.
//

#ifndef LFS_DB_UTIL_HPP
#define LFS_DB_UTIL_HPP

#include "../../main.hpp"

using namespace std;

template<typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

// mapping of enum to string to get the db_keys for metadata
enum class Md_fields {
    atime, mtime, ctime, uid, gid, mode, inode_no, link_count, size, blocks
};

const std::array<std::string, 10> md_field_map = {
        "_atime"s, "_mtime"s, "_ctime"s, "_uid"s, "_gid"s, "_mode"s, "_inodeno"s, "_lnkcnt"s, "_size"s, "_blkcnt"s
};

bool init_rocksdb();

void optimize_rocksdb(rocksdb::Options& options);

//std::string db_build_dentry_key(const fuse_ino_t inode, const std::string& name);
//
//std::string db_build_dentry_prefix(const fuse_ino_t inode);
//
//std::string db_build_dentry_value(const fuse_ino_t inode, const mode_t mode);
//
//std::string db_build_mdata_key(const fuse_ino_t inode, const std::string& field);
//
//string db_build_mdata_key(const string& inode, const string& field);
//
//std::vector<std::string> db_build_all_mdata_keys(const fuse_ino_t inode);

#endif //LFS_DB_UTIL_HPP
