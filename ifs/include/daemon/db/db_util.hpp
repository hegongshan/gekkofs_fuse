
#ifndef LFS_DB_UTIL_HPP
#define LFS_DB_UTIL_HPP

#include <daemon/adafs_daemon.hpp>

using namespace std;

// mapping of enum to string to get the db_keys for metadata
enum class Md_fields {
    atime, mtime, ctime, uid, gid, mode, inode_no, link_count, size, blocks
};

const std::array<std::string, 10> md_field_map = {
        "_atime"s, "_mtime"s, "_ctime"s, "_uid"s, "_gid"s, "_mode"s, "_inodeno"s, "_lnkcnt"s, "_size"s, "_blkcnt"s
};

bool init_rocksdb();

void optimize_rocksdb(rocksdb::Options& options);

std::string
db_build_metadentry_value(); // TODO this would build a value based on the number of metadata fields that are used in the fs configuration

#endif //LFS_DB_UTIL_HPP
