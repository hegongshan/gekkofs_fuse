//
// Created by evie on 2/23/17.
//

#ifndef FS_FUSE_OPS_H
#define FS_FUSE_OPS_H

#include "main.h"

// file
void adafs_ll_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi);


// directory



// I/O



//



// access



// file system miscellaneous


void adafs_ll_init(void* adafs_data, struct fuse_conn_info* conn);

void adafs_ll_destroy(void* adafs_data);

#endif //FS_FUSE_OPS_H
