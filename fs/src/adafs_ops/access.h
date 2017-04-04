//
// Created by evie on 3/28/17.
//

#ifndef FS_ACCESS_H
#define FS_ACCESS_H

#include "../classes/metadata.h"


int chk_access(const Metadata& md, int mask);

int chk_uid(const Metadata& md);

int chmod(Metadata& md, mode_t mode, const bfs::path& path);

#endif //FS_ACCESS_H
