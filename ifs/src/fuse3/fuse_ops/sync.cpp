//
// Created by draze on 5/7/17.
//

#include "../../../main.hpp"
#include "../fuse_ops.hpp"


/**
 * Flush method
 *
 * This is called on each close() of the opened file.
 *
 * Since file descriptors can be duplicated (dup, dup2, fork), for
 * one open call there may be many flush calls.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * fi->fh will contain the value set by the open method, or will
 * be undefined if the open method didn't set any value.
 *
 * NOTE: the name of the method is misleading, since (unlike
 * fsync) the filesystem is not forced to flush pending writes.
 * One reason to flush data, is if the filesystem wants to return
 * write errors.
 *
 * If the filesystem supports file locking operations (setlk,
 * getlk) it should remove all locks belonging to 'fi->owner'.
 *
 * If this request is answered with an error code of ENOSYS,
 * this is treated as success and future calls to flush() will
 * succeed automatically without being send to the filesystem
 * process.
 *
 * Valid replies:
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param fi file information
 */
void adafs_ll_flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    // TODO to be implemented
    ADAFS_DATA->spdlogger()->debug("adafs_ll_flush() enter: inode {}", ino);
    fuse_reply_err(req, 0);
}