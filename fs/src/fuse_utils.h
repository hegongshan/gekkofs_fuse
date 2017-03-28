//
// Created by draze on 3/19/17.
//

#ifndef FS_FUSE_UTILS_H
#define FS_FUSE_UTILS_H

#include "classes/metadata.h"

int chk_access(const Metadata& md, const int mode);

#endif //FS_FUSE_UTILS_H
