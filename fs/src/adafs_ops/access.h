//
// Created by evie on 3/28/17.
//

#ifndef FS_ACCESS_H
#define FS_ACCESS_H

#include "../classes/metadata.h"


int chk_access(const Metadata& md, int mask);

int chk_uid(const Metadata& md);

int change_access(Metadata& md, mode_t mode, const bfs::path& path);

int change_permissions(Metadata& md, uid_t uid, gid_t gid, const bfs::path& path);

#endif //FS_ACCESS_H
