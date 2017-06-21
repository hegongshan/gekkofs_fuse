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

int write_all_metadata(const Metadata& md, const fuse_ino_t inode);

int read_all_metadata(Metadata& md, const fuse_ino_t inode);

int remove_all_metadata(const fuse_ino_t inode);

int get_metadata(Metadata& md, const fuse_ino_t inode);

int get_attr(struct stat& attr, const fuse_ino_t inode);

void metadata_to_stat(const Metadata& md, struct stat& attr);

int create_node(fuse_req_t& req, struct fuse_entry_param& fep, fuse_ino_t parent, const string& name, mode_t mode);

#endif //FS_METADATA_OPS_H
