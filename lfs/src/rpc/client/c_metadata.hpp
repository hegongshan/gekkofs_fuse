//
// Created by evie on 6/22/17.
//

#ifndef LFS_C_METADATA_HPP
#define LFS_C_METADATA_HPP

#include "../rpc_types.hpp"

void send_minimal_rpc(void* arg);

bool rpc_send_create(const uint64_t recipient, const fuse_ino_t parent, const std::string& name,
                     const uid_t uid, const gid_t gid, const mode_t mode, fuse_ino_t& new_inode);

#endif //LFS_C_METADATA_HPP
