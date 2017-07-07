//
// Created by evie on 6/22/17.
//

#ifndef LFS_C_METADATA_HPP
#define LFS_C_METADATA_HPP

#include "../rpc_types.hpp"

void send_minimal_rpc(void* arg);

int rpc_send_create_dentry(const size_t recipient, const fuse_ino_t parent, const std::string& name,
                           const mode_t mode, fuse_ino_t& new_inode);

int rpc_send_create_mdata(const size_t recipient, const uid_t uid, const gid_t gid,
                          const mode_t mode, const fuse_ino_t inode);

int rpc_send_create(const size_t recipient, const fuse_ino_t parent, const std::string& name,
                    const uid_t uid, const gid_t gid, const mode_t mode, fuse_ino_t& new_inode);

int rpc_send_get_attr(const size_t recipient, const fuse_ino_t inode, struct stat& attr);

#endif //LFS_C_METADATA_HPP
