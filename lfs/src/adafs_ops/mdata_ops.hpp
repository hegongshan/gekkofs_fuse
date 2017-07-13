//
// Created by draze on 3/5/17.
//

#ifndef FS_METADATA_OPS_H
#define FS_METADATA_OPS_H

#include "../main.hpp"
#include "../classes/metadata.hpp"
#include "../db/db_ops.hpp"
#include "../db/db_txn_ops.hpp"
#include "../db/db_util.hpp"

using namespace std;

/**
 * Reads a specific metadata field to the database
 * @tparam T
 * @param inode
 * @param field
 * @return type, 0 might mean failure
 */
template<typename T>
decltype(auto) read_metadata_field(const fuse_ino_t inode, Md_fields field) {
    // XXX I am sure this can be implemented in a better way
    switch (field) {
        case Md_fields::atime:
            return db_get_mdata<T>(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::atime)>(md_field_map)));
        case Md_fields::mtime:
            return db_get_mdata<T>(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::mtime)>(md_field_map)));
        case Md_fields::ctime:
            return db_get_mdata<T>(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::ctime)>(md_field_map)));
        case Md_fields::uid:
            return db_get_mdata<T>(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::uid)>(md_field_map)));
        case Md_fields::gid:
            return db_get_mdata<T>(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::gid)>(md_field_map)));
        case Md_fields::mode:
            return db_get_mdata<T>(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::mode)>(md_field_map)));
        case Md_fields::inode_no:
            return db_get_mdata<T>(
                    db_build_mdata_key(inode, std::get<to_underlying(Md_fields::inode_no)>(md_field_map)));
        case Md_fields::link_count:
            return db_get_mdata<T>(
                    db_build_mdata_key(inode, std::get<to_underlying(Md_fields::link_count)>(md_field_map)));
        case Md_fields::size:
            return db_get_mdata<T>(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::size)>(md_field_map)));
        case Md_fields::blocks:
            return db_get_mdata<T>(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::blocks)>(md_field_map)));
    }

}

/**
 * writes a specific metadata field to the database
 * @tparam T
 * @param inode
 * @param field
 * @param md
 * @return bool - success
 */
template<typename T>
bool write_metadata_field(const fuse_ino_t inode, const Md_fields field, Metadata& md) {
    // XXX I am sure this can be implemented in a better way
    switch (field) {
        case Md_fields::atime:
            return db_put_mdata(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::atime)>(md_field_map)),
                                md.atime());
        case Md_fields::mtime:
            return db_put_mdata(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::mtime)>(md_field_map)),
                                md.mtime());
        case Md_fields::ctime:
            return db_put_mdata(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::ctime)>(md_field_map)),
                                md.ctime());
        case Md_fields::uid:
            return db_put_mdata(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::uid)>(md_field_map)),
                                md.uid());
        case Md_fields::gid:
            return db_put_mdata(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::gid)>(md_field_map)),
                                md.gid());
        case Md_fields::mode:
            return db_put_mdata(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::mode)>(md_field_map)),
                                md.mode());
        case Md_fields::inode_no:
            return db_put_mdata(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::inode_no)>(md_field_map)),
                                md.inode_no());
        case Md_fields::link_count:
            return db_put_mdata(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::link_count)>(md_field_map)),
                                md.link_count());
        case Md_fields::size:
            return db_put_mdata(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::size)>(md_field_map)),
                                md.size());
        case Md_fields::blocks:
            return db_put_mdata(db_build_mdata_key(inode, std::get<to_underlying(Md_fields::blocks)>(md_field_map)),
                                md.blocks());
    }
}


int write_all_metadata(const Metadata& md);

int read_all_metadata(Metadata& md, const fuse_ino_t inode);

int remove_all_metadata(const fuse_ino_t inode);

int get_metadata(Metadata& md, const fuse_ino_t inode);

int get_attr(struct stat& attr, const fuse_ino_t inode);

void metadata_to_stat(const Metadata& md, struct stat& attr);

int init_metadata_fep(struct fuse_entry_param& fep, const fuse_ino_t inode, const uid_t uid, const gid_t gid,
                      mode_t mode);

int init_metadata(const fuse_ino_t inode, const uid_t uid, const gid_t gid, mode_t mode);

int create_node(struct fuse_entry_param& fep, fuse_ino_t parent, const char* name, const uid_t uid, const gid_t gid,
                mode_t mode);

int remove_node(fuse_ino_t parent, const char* name);

#endif //FS_METADATA_OPS_H
