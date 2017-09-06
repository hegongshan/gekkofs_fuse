//
// Created by evie on 9/6/17.
//

#include "adafs_ops/metadentry.hpp"
#include "db/db_ops.hpp"

int create_node(const std::string& path, const uid_t uid, const gid_t gid, mode_t mode) {
    create_metadentry(path, mode); // XXX errorhandling

    // TODO create chunk space for data

    return 0;
}

int create_metadentry(const std::string& path, mode_t mode) {
    auto val = fmt::FormatInt(
            mode).str(); // For now just the mode is the value. Later any metadata combination can be in there. TODO
    return db_put_metadentry(path, val);
}

int get_attr(const std::string& path, struct stat& attr) {
    // TODO make metadata object with dummy values. Inode is hashed pathname
//    attr.st_ino = md.inode_no();
//    attr.st_mode = md.mode();
//    attr.st_nlink = md.link_count();
//    attr.st_uid = md.uid();
//    attr.st_gid = md.gid();
//    attr.st_size = md.size();
//    attr.st_blksize = ADAFS_DATA->blocksize(); // globally set blocksize is used
//    attr.st_blocks = md.blocks();
//    attr.st_atim.tv_sec = md.atime();
//    attr.st_mtim.tv_sec = md.mtime();
//    attr.st_ctim.tv_sec = md.ctime();
}