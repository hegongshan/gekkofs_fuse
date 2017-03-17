//
// Created by evie on 3/17/17.
//

#include "../main.h"
#include "../fuse_ops.h"

int adafs_opendir(const char* p, struct fuse_file_info* fi) {
    return 0;
}

int adafs_readdir(const char* p, void* buf, fuse_fill_dir_t filler, off_t offset,
                  struct fuse_file_info* fi, enum fuse_readdir_flags flags) {
    ADAFS_DATA->logger->info("bb_readdir(path=\"{}\", buf={}, offset={}", p, buf, offset);
//    ADAFS_DATA->logger->info("bb_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)",
//                             p, buf, filler, offset, fi);
    return 0;
}

int adafs_releasedir(const char*, struct fuse_file_info*) {
    return 0;
}