//
// Created by evie on 2/23/17.
//

#ifndef FS_FUSE_OPS_H
#define FS_FUSE_OPS_H

#include "main.h"
#include "fuse_utils.h"

// file
int adafs_getattr(const char*, struct stat*, struct fuse_file_info*);

int adafs_mknod(const char*, mode_t, dev_t);

int adafs_open(const char*, struct fuse_file_info*);

int adafs_utimens(const char*, const struct timespec tv[2], struct fuse_file_info* fi);

// directory
int adafs_opendir(const char*, struct fuse_file_info*);

int adafs_readdir(const char*, void*, fuse_fill_dir_t, off_t, struct fuse_file_info*, enum fuse_readdir_flags);

int adafs_releasedir(const char*, struct fuse_file_info*);

int adafs_mkdir(const char *, mode_t);

//
int adafs_flush(const char*, struct fuse_file_info*);

//


void* adafs_init(struct fuse_conn_info*, struct fuse_config*);

void adafs_destroy(void*);

#endif //FS_FUSE_OPS_H
