//
// Created by evie on 4/7/17.
//

#include "../main.h"
#include "../fuse_ops.h"
#include "../adafs_ops/metadata_ops.h"
#include "../adafs_ops/dentry_ops.h"
#include "../adafs_ops/access.h"

#include "../classes/dentry.h"

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


    //get inode no first (either from cache or disk) with parent inode and name
    auto inode = do_lookup(req, parent, string(name));
    if (inode < 1) {
        fuse_reply_err(req, static_cast<int>(inode));
        return;
    }

    auto fep = make_shared<struct fuse_entry_param>();
    get_attr(fep->attr, inode);
    fep->ino = fep->attr.st_ino;
    fep->entry_timeout = 1.0;
    fep->attr_timeout = 1.0;

    fuse_reply_entry(req, fep.get());


    /* for ENOENTs
     * if (err == -ENOENT && f->conf.negative_timeout != 0.0) {
			e.ino = 0;
			e.entry_timeout = f->conf.negative_timeout;
			err = 0;
		}
     */

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
#ifdef CHECK_ACCESS //TODO
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

    // XXX this'll break for large dirs as only the entries are shown that can fit in the buffer TODO later
    if (off == 0) {
        auto off_cnt = off;
        // XXX can this be done in c++11 style?
//        auto buf2 = make_unique<array<char, size>>();
//        auto p2 = make_shared<char>();
        char* buf = new char[size](); // buffer, holding the dentries
        char* p; // pointer pointing on the current buffer entry
        size_t remaining_size; // remaining available size

//        p2 = buf2;
        p = buf;
        remaining_size = size;
        size_t entry_size;

        auto dentries = make_shared<vector<Dentry>>();

        get_dentries(*dentries, ino);
        //getdentries here
        for (const auto& dentry : *dentries) {
            ADAFS_DATA->spdlogger()->trace("readdir dentry: name {} inode {} mode {:o}", dentry.name(), dentry.inode(), dentry.mode());

            /*
             * Generate tiny stat with inode and mode information.
             * This information is necessary so that the entry shows up later at all.
             * The inode and mode information does not seem to impact correctness since lookup is setting
             * the correct inode again. However, it is unclear if the kernel uses this information for optimizations.
             */
            auto attr = make_unique<struct stat>();
            attr->st_ino = dentry.inode();
            attr->st_mode = dentry.mode(); // only bits 12-15 are used (i.e., file type)
            // add directory entry giving it to fuse and getting the actual entry size information
            entry_size = fuse_add_direntry(req, p, remaining_size, dentry.name().c_str(), attr.get(), ++off_cnt);

            if (entry_size > remaining_size)
                break;

            p += entry_size; // move pointer
            remaining_size -= entry_size; // subtract entry size from remaining size
        }

        fuse_reply_buf(req, buf, size - remaining_size);
        free(buf);

    } else {
        // return no entry, i.e., finished with readdir
        char* buf = new char[size]();
        size_t remaining_size = size;
        fuse_reply_buf(req, buf, size - remaining_size);
        free(buf);
    }
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