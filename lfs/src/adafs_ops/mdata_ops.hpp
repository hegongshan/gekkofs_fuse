//
// Created by draze on 3/5/17.
//

#ifndef FS_METADATA_OPS_H
#define FS_METADATA_OPS_H

#include "../main.hpp"
#include "../classes/metadata.h"
#include "db_ops.hpp"

using namespace std;

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

// mapping of enum to string to get the file names for metadata
enum class Md_fields { atime, mtime, ctime, uid, gid, mode, inode_no, link_count, size, blocks };

const std::array<std::string, 10> md_field_map = {
        "_atime"s, "_mtime"s, "_ctime"s, "_uid"s, "_gid"s, "_mode"s, "_inodeno"s, "_lnkcnt"s, "_size"s, "_blkcnt"s
};

int write_all_metadata(const Metadata& md, const fuse_ino_t inode);

int read_all_metadata(Metadata& md, const fuse_ino_t inode);

int remove_all_metadata(const fuse_ino_t inode);

int get_metadata(Metadata& md, const fuse_ino_t inode);

int get_attr(struct stat& attr, const fuse_ino_t inode);

void metadata_to_stat(const Metadata& md, struct stat& attr);

int create_node(fuse_req_t& req, struct fuse_entry_param& fep, fuse_ino_t parent, const string& name, mode_t mode);

#endif //FS_METADATA_OPS_H
