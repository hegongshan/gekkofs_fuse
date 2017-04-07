//
// Created by evie on 4/7/17.
//

#include "../main.h"
#include "../fuse_ops.h"

using namespace std;

/**
 * Look up a directory entry by name and get its attributes.
 *
 * Valid replies:
 *   fuse_reply_entry
 *   fuse_reply_err
 *
 * @param req request handle
 * @param parent inode number of the parent directory
 * @param name the name to look up
 */
void adafs_ll_lookup(fuse_req_t req, fuse_ino_t parent, const char* name) {
    spdlogger->debug("adafs_ll_lookup() enter: parent_inode {} name \"{}\"", parent, name);

    fuse_reply_err(req, ENOENT);
}