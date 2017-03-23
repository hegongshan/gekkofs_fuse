//
// Created by evie on 3/17/17.
//

#include "../main.h"
#include "../fuse_ops.h"
#include "../metadata_ops.h"

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
    ADAFS_DATA->logger->info("FUSE: adafs_opendir() enter"s);
    // XXX error handling
    auto path = bfs::path(p);
    auto md = make_shared<Metadata>();

    get_metadata(*md, path);

    int access = fi->flags & O_ACCMODE;

//    ADAFS_DATA->logger->info("access variable: {}", access);
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
    ADAFS_DATA->logger->info("FUSE: adafs_readdir() enter"s);
//    ADAFS_DATA->logger->info("FUSE: adafs_readdir(path=\"{}\", buf={}, offset={}", p, buf, offset);
//    ADAFS_DATA->logger->info("bb_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)",
//                             p, buf, filler, offset, fi);
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

    for (auto& dentry : *dentries) {
        // XXX I have no idea what the last parameter really does...
        filler(buf, dentry.c_str(), NULL, 0, FUSE_FILL_DIR_PLUS);
    }

    return 0;
}

int adafs_releasedir(const char*, struct fuse_file_info*) {
    return 0;
}