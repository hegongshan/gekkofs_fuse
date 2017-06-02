//
// Created by evie on 5/12/17.
//

#include "../main.hpp"
#include "../fuse_ops.hpp"

/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 * This method is not called under Linux kernel versions 2.4.x
 *
 * If this request is answered with an error code of ENOSYS, this is
 * treated as a permanent success, i.e. this and all future access()
 * requests will succeed without being send to the filesystem process.
 *
 * Valid replies:
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param mask requested access mode
 */
void adafs_ll_access(fuse_req_t req, fuse_ino_t ino, int mask) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_access() enter: ino {} mask {}", ino, mask);

    fuse_reply_err(req, 0);
}