//
// Created by evie on 4/6/17.
//

#include "../main.h"
#include "../fuse_ops.h"
#include "../adafs_ops/metadata_ops.h"
#include "../adafs_ops/dentry_ops.h"
#include "../adafs_ops/access.h"
#include "../adafs_ops/io.h"

using namespace std;

/**
 * Get file attributes.
 *
 * If writeback caching is enabled, the kernel may have a
 * better idea of a file's length than the FUSE file system
 * (eg if there has been a write that extended the file size,
 * but that has not yet been passed to the filesystem.n
 *
 * In this case, the st_size value provided by the file system
 * will be ignored.
 *
 * Valid replies:
 *   fuse_reply_attr
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param fi for future use, currently always NULL
 */
void adafs_ll_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_getattr() enter: inode {}", ino);
    auto attr = make_unique<struct stat>();
    auto md = make_shared<Metadata>();
    auto err = get_metadata(*md, ino);
    if (!err) {
        attr->st_ino = md->inode_no();
        attr->st_mode = md->mode();
        attr->st_nlink = md->link_count();
        attr->st_uid = md->uid();
        attr->st_gid = md->gid();
        attr->st_size = md->size();
        attr->st_blksize = ADAFS_DATA->blocksize();
        attr->st_blocks = md->blocks();
        attr->st_atim.tv_sec = md->atime();
        attr->st_mtim.tv_sec = md->mtime();
        attr->st_ctim.tv_sec = md->ctime();
        // XXX take a look into timeout value later
        fuse_reply_attr(req, attr.get(), 1.0);
    } else {
        fuse_reply_err(req, err);
    }
}