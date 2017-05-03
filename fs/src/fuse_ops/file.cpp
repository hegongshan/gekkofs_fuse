//
// Created by evie on 3/17/17.
//

#include "../main.h"
#include "../fuse_ops.h"
#include "../adafs_ops/metadata_ops.h"
#include "../adafs_ops/dentry_ops.h"
#include "../adafs_ops/access.h"
#include "../adafs_ops/io.h"

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
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_getattr() enter: name '{}' initial_inode {}", p,
                              attr->st_ino);
    auto path = bfs::path(p);
    auto md = make_shared<Metadata>();

// uncomment when mdtest.files should return 0 without actually checking if its there
//    if (path.filename().string().find("file.mdtest") == 0) {
//        auto md = make_unique<Metadata>(S_IFREG | 664);
//        md->init_ACM_time();
//        attr->st_ino = md->inode_no();
//        attr->st_mode = md->mode();
//        attr->st_nlink = md->link_count();
//        attr->st_uid = md->uid();
//        attr->st_gid = md->gid();
//        attr->st_size = md->size();
//        attr->st_blksize = ADAFS_DATA->blocksize;
//        attr->st_blocks = md->blocks();
//        attr->st_atim.tv_sec = md->atime();
//        attr->st_mtim.tv_sec = md->mtime();
//        attr->st_ctim.tv_sec = md->ctime();
//        return 0;
//    }

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
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_mknod() enter: name '{}' mode {} dev {}", p, mode, dev);
    // XXX Errorhandling and beware of transactions. saving dentry and metadata have to be atomic
    auto path = bfs::path(p);
// uncomment if file creates done with mdtest should return immediately without creating the file
//    if (path.filename().string().find("file.mdtest") == 0)
//        return 0;

    // XXX check if file exists (how can we omit this? Let's just try to create it and see if it fails)

    // XXX check permissions (omittable)

    // create directory entry (can fail)
    create_dentry(ADAFS_DATA->hashf(path.parent_path().string()), path.filename().string());

    // create metadata of new file
    // mode is used here to init metadata
    auto md = make_unique<Metadata>(S_IFREG | mode);
    write_all_metadata(*md, ADAFS_DATA->hashf(path.string()));

    // create chunk space
    init_chunk_space(ADAFS_DATA->hashf(path.string()));

    return 0;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 */
int adafs_create(const char* p, mode_t mode, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_create() enter: name '{}' mode {}", p, mode);

    auto err = static_cast<int>(adafs_mknod(p, mode, 0));
    if (err != 0) return err;
#ifdef CHECK_ACCESS
    err = adafs_open(p, fi);
#endif
    return err;
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
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_open() enter: name '{}'", p);
#ifdef CHECK_ACCESS
    // XXX error handling
    auto path = bfs::path(p);
// uncomment if file creates done with mdtest should return immediately without creating the file
//    if (path.filename().string().find("file.mdtest") == 0)
//        return 0;
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
#else
    return 0;
#endif
}

/** Remove a file */
// XXX Errorhandling err doesnt really work with fuse...
int adafs_unlink(const char* p) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_unlink() enter: name '{}'", p);
    auto path = bfs::path(p);

    // XXX I don't think we need to check if path.filename() is not root. Because unlink is only called with files

    // XXX consider hardlinks

    // adafs_access was called first by the VFS. Thus, path exists and access is ok (not guaranteed though!).
    auto err = remove_dentry(ADAFS_DATA->hashf(path.parent_path().string()), path.filename().string());
    if (err != 0) return err;

    // remove inode
    err = remove_metadata(ADAFS_DATA->hashf(path.string()));
    if (err != 0) return err;

    // XXX delete unused data blocks (asynchronously)
    // XXX currently just throws away the data directory on disk
    destroy_chunk_space(ADAFS_DATA->hashf(path.string()));

    return 0;
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
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_utimens() enter: name '{}'", p);
    // XXX ignored for now. Later: Make it configurable
    return 0;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.	 It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 */
int adafs_release(const char* p, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_release() enter: name '{}'", p);
    // XXX Dunno what this function is for yet
    return 0;
}
