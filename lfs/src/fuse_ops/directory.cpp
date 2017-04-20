//
// Created by evie on 4/7/17.
//

#include "../main.h"
#include "../fuse_ops.h"
#include "../adafs_ops/metadata_ops.h"
#include "../adafs_ops/access.h"

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
    ADAFS_DATA->spdlogger()->debug("adafs_ll_lookup() enter: parent_inode {} name \"{}\"", parent, name);

    auto md = make_shared<Metadata>();
    get_metadata(*md, 1);
    auto fep = make_unique<struct fuse_entry_param>();

    fep->entry_timeout = 1.0;
    fep->attr_timeout = 1.0;


    fep->attr.st_mode = md->mode();
    fep->attr.st_nlink = md->link_count();
    fep->attr.st_uid = md->uid();
    fep->attr.st_gid = md->gid();
    fep->attr.st_size = md->size();
    fep->attr.st_blksize = ADAFS_DATA->blocksize();
    fep->attr.st_blocks = md->blocks();
    fep->attr.st_atim.tv_sec = md->atime();
    fep->attr.st_mtim.tv_sec = md->mtime();
    fep->attr.st_ctim.tv_sec = md->ctime();

    if (strcmp(name, "test123") == 0) {
        fep->ino = 2;
        fep->attr.st_ino = 2;
        ADAFS_DATA->spdlogger()->debug("I am inside test123"s);
        fuse_reply_entry(req, fep.get());
    } else if (strcmp(name, "test456") == 0) {
        fep->ino = 3;
        fep->attr.st_ino = 3;
        ADAFS_DATA->spdlogger()->debug("I am inside test456"s);
        fuse_reply_entry(req, fep.get());
    } else {

        fuse_reply_err(req, ENOENT);
    }

}

/**
 * Open a directory
 *
 * Filesystem may store an arbitrary file handle (pointer, index,
 * etc) in fi->fh, and use this in other all other directory
 * stream operations (readdir, releasedir, fsyncdir).
 *
 * Filesystem may also implement stateless directory I/O and not
 * store anything in fi->fh, though that makes it impossible to
 * implement standard conforming directory stream operations in
 * case the contents of the directory can change between opendir
 * and releasedir.
 *
 * Valid replies:
 *   fuse_reply_open
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param fi file information
	 */
void adafs_ll_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_opendir() enter: inode {}", ino);
#ifdef CHECK_ACCESS
    // XXX error handling
    auto md = make_shared<Metadata>();

    auto err = get_metadata(*md, ino);

    if (!err) {
        int access = fi->flags & O_ACCMODE;

//    ADAFS_DATA->logger->debug("access variable: {}", access);
        switch (access) {
            case O_RDONLY:
                err = chk_access(req, *md, R_OK);
                break;
            case O_WRONLY:
                err = chk_access(req, *md, W_OK);
                break;
            case O_RDWR:
                err = chk_access(req, *md, R_OK | W_OK);
                break;
            default:
                err = EACCES;
        }
    }
    if (err)
        fuse_reply_err(req, err);
    else
        fuse_reply_open(req, fi);

#else
    // access permitted without checking
    fuse_reply_open(req, fi);
#endif
}

/**
 * Read directory
 *
 * Send a buffer filled using fuse_add_direntry(), with size not
 * exceeding the requested size.  Send an empty buffer on end of
 * stream.
 *
 * fi->fh will contain the value set by the opendir method, or
 * will be undefined if the opendir method didn't set any value.
 *
 * Returning a directory entry from readdir() does not affect
 * its lookup count.
 *
 * The function does not have to report the '.' and '..'
 * entries, but is allowed to do so.
 *
 * Valid replies:
 *   fuse_reply_buf
 *   fuse_reply_data
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param size maximum number of bytes to send
 * @param off offset to continue reading the directory stream
 * @param fi file information
 */
void adafs_ll_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info* fi) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_readdir() enter: inode {} size {} offset {}", ino, size, off);

//    auto md = make_shared<Metadata>();
    // first check that dir exists that is trying to be read. I don't know if this is actually needed,
    // because opendir should have been called first. I guess it could have been deleted in between
//    auto err = get_metadata(*md, ino);
    // XXX permission check here because ls requires read permission (Do we care?)
    if (ino == 1 && off < 2) {


        char* buf = new char[size](); // buffer, holding the dentries
        char* p; // pointer pointing on the current buffer entry
        size_t remaining_size; // remaining available size

        p = buf;
        remaining_size = size;
        size_t entry_size;
        // Read all filenames in dir entry folder for inode of md
//    auto dentries = make_shared<vector<string>>();

        // # first testentry
        auto st = make_unique<struct stat>();
        st->st_ino = 2;
        st->st_mode = S_IRWXO;
        ADAFS_DATA->spdlogger()->debug("First fuse_add_direntry"s);
        entry_size = fuse_add_direntry(req, p, remaining_size, "test123", st.get(), 1);

        p += entry_size; // move pointer
        remaining_size -= entry_size; // subtract entry size from remaining size

        // # second testentry
        auto st2 = make_unique<struct stat>();
        st2->st_ino = 3;
        st2->st_mode = S_IRWXO;

        ADAFS_DATA->spdlogger()->debug("Second fuse_add_direntry"s);
        fuse_add_direntry(req, p, remaining_size, "test456", st2.get(), 2);

        p += entry_size; // move pointer
        remaining_size -= entry_size; // subtract entry size from remaining size

        fuse_reply_buf(req, buf, size - remaining_size);
        free(buf);
    } else if (off == 2) {
        char* buf = new char[size]();
        size_t remaining_size = size;
        fuse_reply_buf(req, buf, size - remaining_size);
        free(buf);
    } else {
        fuse_reply_err(req, 1);

    }
    return;
}

/**
 * Release an open directory
 *
 * For every opendir call there will be exactly one releasedir
 * call.
 *
 * fi->fh will contain the value set by the opendir method, or
 * will be undefined if the opendir method didn't set any value.
 *
 * Valid replies:
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param fi file information
 */
void adafs_ll_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_releasedir() enter: inode {} ", ino);

    fuse_reply_err(req, 0);
}