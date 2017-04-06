//
// Created by evie on 4/6/17.
//

//#include "../main.h"
//#include "../fuse_ops.h"
//#include "../adafs_ops/metadata_ops.h"
//#include "../adafs_ops/dentry_ops.h"
//#include "../adafs_ops/access.h"
//#include "../adafs_ops/io.h"
//
//using namespace std;
//
///**
// * Get file attributes.
// *
// * If writeback caching is enabled, the kernel may have a
// * better idea of a file's length than the FUSE file system
// * (eg if there has been a write that extended the file size,
// * but that has not yet been passed to the filesystem.n
// *
// * In this case, the st_size value provided by the file system
// * will be ignored.
// *
// * Valid replies:
// *   fuse_reply_attr
// *   fuse_reply_err
// *
// * @param req request handle
// * @param ino the inode number
// * @param fi for future use, currently always NULL
// */
//void adafs_ll_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
//    ADAFS_DATA(req)->logger->debug("##### FUSE FUNC ###### adafs_getattr() enter: inode {}", ino);
//}