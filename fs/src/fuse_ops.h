//
// Created by evie on 2/23/17.
//

#ifndef FS_FUSE_OPS_H
#define FS_FUSE_OPS_H

int adafs_getattr(const char *path, struct stat *attr);
void *adafs_init(struct fuse_conn_info *conn);
void adafs_destroy(void *);
#endif //FS_FUSE_OPS_H
