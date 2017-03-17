//
// Created by evie on 3/17/17.
//

#include "../main.h"
#include "../fuse_ops.h"
#include "../metadata.h"
#include "../metadata_ops.h"

using namespace std;

int adafs_getattr(const char* p, struct stat* attr, struct fuse_file_info* fi) {

    auto path = bfs::path(p);
    auto md = make_shared<Metadata>();

    if (get_metadata(*md, path) != -ENOENT) {
        attr->st_ino = md->inode_no();
        attr->st_mode = md->mode();
        attr->st_nlink = md->link_count();
        attr->st_uid = md->uid();
        attr->st_gid = md->gid();
        attr->st_size = md->size();
        attr->st_blksize = ADAFS_DATA->blocksize;
        attr->st_blocks = md->blocks();
        attr->st_atim.tv_sec = md->atime();
        attr->st_mtim.tv_sec = md->mtime();
        attr->st_ctim.tv_sec = md->ctime();
        return 0;
    }

// XXX Testing stuff below. Tobe removed later when files can be created n stuff
//    if (strcmp(p, "/file") == 0) {
//        attr->st_mode = S_IFDIR | 0755;
//        return 0;
//    }
//    if (strcmp(p, "/file/file2") == 0) {
//        auto p_dir = make_shared<struct stat>();
//        lstat("/", p_dir.get());
//        ADAFS_DATA->logger->info(p_dir->st_ino);
//        ADAFS_DATA->logger->flush();
//
//        attr->st_mode = S_IFREG | 0755;
//        attr->st_nlink = 1;
//        attr->st_size = strlen("blubb");
//        return 0;
//    }
    return -ENOENT;
}