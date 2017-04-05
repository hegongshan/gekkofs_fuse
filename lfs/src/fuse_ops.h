//
// Created by evie on 2/23/17.
//

#ifndef FS_FUSE_OPS_H
#define FS_FUSE_OPS_H

#include "main.h"

// file



// directory



// I/O



//



// access



// file system miscellaneous


void adafs_init(void* adafs_data, struct fuse_conn_info* conn);

void adafs_destroy(void* adafs_data);

#endif //FS_FUSE_OPS_H
