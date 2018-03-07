
#include "../../../main.hpp"
#include "../fuse_ops.hpp"
#include "../adafs_ops/mdata_ops.hpp"
#include "../adafs_ops/dentry_ops.hpp"
#include "../adafs_ops/access.hpp"

#include "../classes/dentry.hpp"
#include "../rpc/client/c_dentry.hpp"


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
    int err;
    fuse_ino_t inode;

    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(RPC_DATA->get_dentry_hashable(parent, name));
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            tie(err, inode) = do_lookup(parent, string(name));
        } else { // remote
            err = rpc_send_lookup(recipient, parent, name, inode);
        }
    } else { // single node operation
        //get inode no first (either from cache or disk) with parent inode and name;; returns <err, inode_of_dentry> pair
        tie(err, inode) = do_lookup(parent, string(name));
    }

    if (err != 0) {
        fuse_reply_err(req, err);
        return;
    }

    struct fuse_entry_param fep{};
    err = get_attr(fep.attr, inode);
    fep.ino = fep.attr.st_ino;
    fep.entry_timeout = 1.0;
    fep.attr_timeout = 1.0;

    if (err == 0)
        fuse_reply_entry(req, &fep);
    else
        fuse_reply_err(req, err);


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
#ifdef CHECK_ACCESS
    auto err = open_chk_access(req, ino, fi->flags);

    if (err != 0)
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
 * Create a directory
 *
 * Valid replies:
 *   fuse_reply_entry
 *   fuse_reply_err
 *
 * @param req request handle
 * @param parent inode number of the parent directory
 * @param name to create
 * @param mode with which to create the new file
 */
void adafs_ll_mkdir(fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_mkdir() enter: p_inode {} name {} mode {:o}", parent, name, mode);

    fuse_entry_param fep{};
    auto err = create_node(fep, parent, name, fuse_req_ctx(req)->uid, fuse_req_ctx(req)->gid, S_IFDIR | mode);

    // return new dentry
    if (err == 0) {
        fuse_reply_entry(req, &fep);
    } else {
        fuse_reply_err(req, err);
    }
}

/**
 * Remove a directory
 *
 * If the directory's inode's lookup count is non-zero, the
 * file system is expected to postpone any removal of the
 * inode until the lookup count reaches zero (see description
 * of the forget function).
 *
 * Valid replies:
 *   fuse_reply_err
 *
 * @param req request handle
 * @param parent inode number of the parent directory
 * @param name to remove
 */
void adafs_ll_rmdir(fuse_req_t req, fuse_ino_t parent, const char* name) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_rmdir() enter: p_inode {} name {}", parent, name);
    // XXX consider the whole lookup count functionality. We need something like a hashtable here, which marks the file
    // XXX see adafs_ll_unlink

    // Below is checking if the directory that should be removed is actually empty. We omit this here
    // and just unlink the directory out of the fs tree.
//    fuse_ino_t inode;
//
//    // get inode of file
//    tie(err, inode) = do_lookup(parent, name);
//    if (err != 0) {
//        fuse_reply_err(req, err);
//        return;
//    }
//
//    // check if dir is empty
//    // TODO THIS IS VEEEEEEEERY SLOW! Doing a RangeScan is not the right approach here!
//    err = is_dir_empty(inode);
//    if (err != 0) {
//        fuse_reply_err(req, err);
//        return;
//    }

    auto err = remove_node(parent, name);

    fuse_reply_err(req, err);
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