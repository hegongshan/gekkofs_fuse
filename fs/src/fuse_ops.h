//
// Created by evie on 2/23/17.
//

#ifndef FS_FUSE_OPS_H
#define FS_FUSE_OPS_H

#include "main.h"

// file
int adafs_getattr(const char* p, struct stat* attr, struct fuse_file_info* fi);

int adafs_mknod(const char* p, mode_t, dev_t);

int adafs_create(const char* p, mode_t mode, struct fuse_file_info* fi);

int adafs_open(const char*, struct fuse_file_info* fi);

int adafs_unlink(const char* p);

int adafs_utimens(const char* p, const struct timespec tv[2], struct fuse_file_info* fi);

int adafs_release(const char* p, struct fuse_file_info* fi);


// directory
int adafs_opendir(const char* p, struct fuse_file_info* fi);

int adafs_readdir(const char* p, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi,
                  enum fuse_readdir_flags flags);

int adafs_releasedir(const char* p, struct fuse_file_info* fi);

int adafs_mkdir(const char* p, mode_t mode);

int adafs_rmdir(const char* p);


// I/O
int adafs_read(const char* p, char* buf, size_t size, off_t offset, struct fuse_file_info* fi);

int adafs_write(const char* p, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi);

int adafs_truncate(const char* p, off_t offset, struct fuse_file_info* fi);


//
int adafs_flush(const char* p, struct fuse_file_info* fi);


// access
int adafs_access(const char* p, int mask);

int adafs_chmod(const char* p, mode_t mode, struct fuse_file_info* fi);

int adafs_chown(const char* p, uid_t uid, gid_t gid, struct fuse_file_info* fi);


// file system miscellaneous
int adafs_statfs(const char* p, struct statvfs* statvfs);


void* adafs_init(struct fuse_conn_info* conn, struct fuse_config* cfg);

void adafs_destroy(void* adafs_data);

#endif //FS_FUSE_OPS_H
