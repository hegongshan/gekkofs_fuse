//
// Created by evie on 3/28/17.
//

#ifndef FS_ACCESS_H
#define FS_ACCESS_H

#include "../classes/metadata.hpp"

int open_chk_access(fuse_req_t& req, fuse_ino_t ino, int flags);

int chk_access(const fuse_req_t& req, const Metadata& md, int mask);

int chk_uid(const fuse_req_t& req, const Metadata& md);

int change_access(Metadata& md, mode_t mode, const bfs::path& path);

int change_permissions(Metadata& md, uid_t uid, gid_t gid, const bfs::path& path);

#endif //FS_ACCESS_H
