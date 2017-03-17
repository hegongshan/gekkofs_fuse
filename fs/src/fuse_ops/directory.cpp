//
// Created by evie on 3/17/17.
//

#include "../main.h"
#include "../fuse_ops.h"

int adafs_opendir(const char*, struct fuse_file_info*) {
    return 0;
}

int adafs_readdir(const char*, void*, fuse_fill_dir_t, off_t, struct fuse_file_info*, enum fuse_readdir_flags) {
    return 0;
}

int adafs_releasedir(const char*, struct fuse_file_info*) {
    return 0;
}