//
// Created by evie on 2/23/17.
//

#ifndef FS_FUSE_OPS_H
#define FS_FUSE_OPS_H

#include "main.h"

// file
void adafs_ll_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi);
void adafs_ll_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi);
void adafs_ll_create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi);
void adafs_ll_mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev);

void adafs_ll_unlink(fuse_req_t req, fuse_ino_t p_inode, const char *name);
void adafs_ll_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
void adafs_ll_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

// directory
void adafs_ll_lookup(fuse_req_t req, fuse_ino_t parent, const char* name);
void adafs_ll_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi);
void adafs_ll_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info* fi);
void adafs_ll_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi);
// I/O



// sync
void adafs_ll_flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);


// access
void adafs_ll_access(fuse_req_t req, fuse_ino_t ino, int mask);


// file system miscellaneous


void adafs_ll_init(void* adafs_data, struct fuse_conn_info* conn);
void adafs_ll_destroy(void* adafs_data);

#endif //FS_FUSE_OPS_H
