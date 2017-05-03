//
// Created by evie on 3/17/17.
//

#include "../main.h"
#include "../fuse_ops.h"
#include "../adafs_ops/metadata_ops.h"
#include "../adafs_ops/dentry_ops.h"
#include "../adafs_ops/access.h"

using namespace std;

/** Open directory
 *
 * Unless the 'default_permissions' mount option is given,
 * this method should check if opendir is permitted for this
 * directory. Optionally opendir may also return an arbitrary
 * filehandle in the fuse_file_info structure, which will be
 * passed to readdir, closedir and fsyncdir.
 */
int adafs_opendir(const char* p, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_opendir() enter: name '{}'", p);
#ifdef CHECK_ACCESS
    // XXX error handling
    auto path = bfs::path(p);
    auto md = make_shared<Metadata>();

    get_metadata(*md, path);

    int access = fi->flags & O_ACCMODE;

//    ADAFS_DATA->logger->debug("access variable: {}", access);
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

/** Read directory
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * ### We use version 1 ###
 */
int adafs_readdir(const char* p, void* buf, fuse_fill_dir_t filler, off_t offset,
                  struct fuse_file_info* fi, enum fuse_readdir_flags flags) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_readdir() enter: name {} readdir_flags '{}'", p, flags);
    // XXX ls also reports the number of allocated blocks IN the directory. Non recursive. Currently not considered


    auto path = bfs::path(p);
    auto md = make_shared<Metadata>();
    // first check that dir exists that is trying to be read. I don't know if this is actually needed,
    // because opendir should have been called first. I guess it could have been deleted in between
    get_metadata(*md, path);

    // XXX permission check here because ls requires read permission (Do we care?)

    // Read all filenames in dir entry folder for inode of md
    auto dentries = make_shared<vector<string>>();
    if (read_dentries(*dentries, ADAFS_DATA->hashf(path.string())) != 0)
        return 1; // XXX problemo dedected deal with it later (I mean me)

    // visualizing current and parent dir
    filler(buf, ".", nullptr, 0, FUSE_FILL_DIR_PLUS);
    filler(buf, "..", nullptr, 0, FUSE_FILL_DIR_PLUS);
    for (const auto& dentry : *dentries) {
        // XXX I have no idea what the last parameter really does...
        ADAFS_DATA->logger->debug("readdir entries: dentry: {}", dentry);
        filler(buf, dentry.c_str(), nullptr, 0, FUSE_FILL_DIR_PLUS);
    }

    return 0;
}

/** Release directory
 */
int adafs_releasedir(const char* p, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_releasedir() enter: name '{}'", p);
    // XXX Dunno what to do with that function yet. Maybe flush dirty dentries that are in cache?
    // At the time of this writing I don't have any cache running. So all dirty stuff is immediately written to disk.
    return 0;
}

/** Create a directory
 *
 * Note that the mode argument may not have the type specification
 * bits set, i.e. S_ISDIR(mode) can be false.  To obtain the
 * correct directory type bits use  mode|S_IFDIR
 * */
int adafs_mkdir(const char* p, mode_t mode) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_mkdir() enter: name '{}' mode {}", p, mode);
    // XXX mknod and mkdir is strikingly similar. todo merge them.
    // XXX Errorhandling and beware of transactions. saving dentry and metadata have to be atomic
    auto path = bfs::path(p);
    path.remove_trailing_separator();

    // XXX check if dir exists (how can we omit this? Let's just try to create it and see if it fails)

    // XXX check permissions (omittable)

    // create directory entry for parent directory (can fail)
    create_dentry(ADAFS_DATA->hashf(path.parent_path().string()), path.filename().string());

    // create metadata of new file
    // mode is used here to init metadata
    auto md = make_unique<Metadata>(S_IFDIR | mode);
    md->size(4096); // XXX just visual. size computation of directory should be done properly at some point
    write_all_metadata(*md, ADAFS_DATA->hashf(path.string()));

    // Init structure to hold dentries of new directory
    init_dentry_dir(ADAFS_DATA->hashf(path.string()));

    return 0;
}

/** Remove a directory. Has to be empty */
// XXX errorhandling err doesnt really work with fuse...
int adafs_rmdir(const char* p) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_rmdir() enter: name '{}'", p);
    auto path = bfs::path(p);

    // check that directory is empty
    if (!is_dir_empty(path)) return -ENOTEMPTY;

    // remove dentry XXX duplicate code in adafs_unlink()
    auto err = remove_dentry(ADAFS_DATA->hashf(path.parent_path().string()), path.filename().string());
    if (err != 0) return err;

    // remove dentry directory
    destroy_dentry_dir(ADAFS_DATA->hashf(path.string()));

    // remove directory inode
    err = remove_metadata(ADAFS_DATA->hashf(path.string()));
    if (err != 0) return err;

    return 0;
}
