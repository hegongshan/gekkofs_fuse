//
// Created by evie on 3/17/17.
//

#include "../main.h"
#include "../fuse_ops.h"
#include "../adafs_ops/metadata_ops.h"
#include "../adafs_ops/dentry_ops.h"

using namespace std;

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored. The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 *
 * `fi` will always be NULL if the file is not currenly open, but
 * may also be NULL if the file is open.
 */
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

    return -ENOENT;
}

/** Create a file node
 *
 * This is called for creation of all non-directory, non-symlink
 * nodes.  If the filesystem defines a create() method, then for
 * regular files that will be called instead.
 */
int adafs_mknod(const char* p, mode_t mode, dev_t dev) {
    ADAFS_DATA->logger->debug("FUSE: adafs_readdir() enter"s);
    // XXX Errorhandling and beware of transactions. saving dentry and metadata have to be atomic
    auto path = bfs::path(p);

    // XXX check if file exists (how can we omit this? Let's just try to create it and see if it fails)

    // XXX check permissions (omittable)

    // XXX create directory entry (can fail)
    create_dentry(ADAFS_DATA->hashf(path.parent_path().string()), path.filename().string());

    // XXX create metadata of new file
    // mode is used here to init metadata
    auto md = make_unique<Metadata>(mode);
    write_all_metadata(*md, ADAFS_DATA->hashf(path.string()));

    return 0;
}

/** File open operation
 *
 * No creation (O_CREAT, O_EXCL) and by default also no
 * truncation (O_TRUNC) flags will be passed to open(). If an
 * application specifies O_TRUNC, fuse first calls truncate()
 * and then open(). Only if 'atomic_o_trunc' has been
 * specified and kernel version is 2.6.24 or later, O_TRUNC is
 * passed on to open.
 *
 * Unless the 'default_permissions' mount option is given,
 * open should check if the operation is permitted for the
 * given flags. Optionally open may also return an arbitrary
 * filehandle in the fuse_file_info structure, which will be
 * passed to all file operations.
 */
int adafs_open(const char* p, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->debug("FUSE: adafs_open() enter"s);
    // XXX error handling
    auto path = bfs::path(p);
    auto md = make_shared<Metadata>();

    get_metadata(*md, path);

    int access = fi->flags & O_ACCMODE;

    switch (access) {
        case O_RDONLY:
            return chk_access(*md, R_OK);
        case O_WRONLY:
            return chk_access(*md, W_OK);
        case O_RDWR:
            return chk_access(*md, R_OK | W_OK);
        default:
            return -EACCES;
    }
}

/**
 * Change the access and modification times of a file with
 * nanosecond resolution
 *
 * This supersedes the old utime() interface.  New applications
 * should use this.
 *
 * `fi` will always be NULL if the file is not currenly open, but
 * may also be NULL if the file is open.
 *
 * See the utimensat(2) man page for details.
 */
int adafs_utimens(const char* p, const struct timespec tv[2], struct fuse_file_info* fi) {
    // XXX ignored for now. Later: Make it configurable
    return 0;
}
