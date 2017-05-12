//
// Created by evie on 3/17/17.
//

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include "dentry_ops.h"

#include "../classes/dentry.h"

using namespace std;

/**
 * Initializes the dentry directory to hold future dentries
 * @param inode
 * @return
 */
bool init_dentry_dir(const uint64_t inode) {
    auto d_path = bfs::path(ADAFS_DATA->dentry_path());
    d_path /= to_string(inode);
    bfs::create_directories(d_path);
    // XXX This might not be needed as it is another access to the underlying file system
    return bfs::exists(d_path);
}

/**
 * Destroys the dentry directory
 * @param inode
 * @return true if successfully deleted
 */
bool destroy_dentry_dir(const uint64_t inode) {
    auto d_path = bfs::path(ADAFS_DATA->dentry_path());
    d_path /= to_string(inode);

    // remove dentry dir
    bfs::remove_all(d_path);

    return !bfs::exists(d_path);
}

/**
 * Check if the file name can be found in the directory entries of parent_dir_hash
 * @param parent_dir_hash
 * @param fname
 * @return
 */
bool verify_dentry(const uint64_t inode) {
    // XXX do I need this?
    return false;
//    auto d_path = bfs::path(ADAFS_DATA->dentry_path());
//    if (inode != ADAFS_ROOT_INODE) { // non-root
//        d_path /=
//    }
//
//
//    if (inode.has_parent_path()) { // non-root
//        d_path /= to_string(ADAFS_DATA->hashf(inode.parent_path().string()));
//        d_path /= inode.filename(); // root
//    } else {
//        d_path /= to_string(ADAFS_DATA->hashf(inode.string()));
//    }
//    // if file path exists leaf name is a valid dentry of parent_dir
//    return bfs::exists(d_path);
}


/**
 * Reads all directory entries in a directory with a given @hash. Returns 0 if successful.
 * @dir is assumed to be empty
 */
int read_dentries(const uint64_t p_inode, const unsigned long inode) {
//    auto path = bfs::path(ADAFS_DATA->dentry_path());
//    path /= to_string(inode);
//    if (!bfs::exists(path)) return 1;
//    // shortcut if path is empty = no files in directory
//    if (bfs::is_empty(path)) return 0;
//
//    // Below can be simplified with a C++11 range based loop? But how? :( XXX
//    bfs::directory_iterator end_dir_it;
//    for (bfs::directory_iterator dir_it(path); dir_it != end_dir_it; ++dir_it) {
//        const bfs::path cp = (*dir_it);
//        p_inode.push_back(cp.filename().string());
//    }
    return 0;
}

/**
 * Reads all directory entries in a directory for the given inode. Returns 0 if successful. The dentries vector is not
 * cleared before it is used.
 * @param dentries assumed to be empty
 * @param dir_inode
 * @return
 */
int get_dentries(vector<Dentry>& dentries, const uint64_t dir_inode) {
    ADAFS_DATA->spdlogger()->debug("get_dentries: inode {}", dir_inode);
    auto d_path = bfs::path(ADAFS_DATA->dentry_path());
    d_path /= to_string(dir_inode);
    // shortcut if path is empty = no files in directory
    if (bfs::is_empty(d_path)) return 0;

    auto dir_it = bfs::directory_iterator(d_path);
    for (const auto& it : dir_it) {
        auto dentry = make_shared<Dentry>(it.path().filename().string());
        // retrieve inode number and mode in dentry
        uint64_t inode;
        mode_t mode;
        d_path /= it.path().filename();
        bfs::ifstream ifs{d_path};
        boost::archive::binary_iarchive ba(ifs);
        ba >> inode;
        ba >> mode;
        dentry->inode(inode);
        dentry->mode(mode);
        // append dentry to dentries vector
        dentries.push_back(*dentry);
        d_path.remove_filename();
    }

    return 0;
}

/**
 * Gets the inode of a directory entry
 * @param req
 * @param parent_inode
 * @param name
 * @return inode
 */
uint64_t do_lookup(fuse_req_t& req, const uint64_t p_inode, const string& name) {

    uint64_t inode;
    // XXX error handling
    // TODO look into cache first
    auto d_path = bfs::path(ADAFS_DATA->dentry_path());
    d_path /= to_string(p_inode);
    // XXX check if this is needed later
    d_path /= name;
    if (!bfs::exists(d_path))
        return static_cast<uint64_t>(-ENOENT);

    bfs::ifstream ifs{d_path};
    //read inode from disk
    boost::archive::binary_iarchive ba(ifs);
    ba >> inode;
    ADAFS_DATA->spdlogger()->debug("do_lookup: p_inode {} name {} resolved_inode {}", p_inode, name, inode);

    return inode;
}


/**
 * Creates an empty file in the dentry folder of the parent directory, acting as a dentry for lookup
 * @param parent_dir
 * @param name
 * @return
 */
int create_dentry(const uint64_t p_inode, const uint64_t inode, const string& name, mode_t mode) {
//    ADAFS_DATA->logger->debug("create_dentry() enter with fname: {}", inode);
    // XXX Errorhandling
    auto d_path = bfs::path(ADAFS_DATA->dentry_path());
    d_path /= to_string(p_inode);
    // XXX check if this is needed later
//    if (!bfs::exists(d_path)) return -ENOENT;

    d_path /= name;

    bfs::ofstream ofs{d_path};
    // write to disk in binary form
    boost::archive::binary_oarchive ba(ofs);
    ba << inode;
    ba << mode;

    // XXX make sure the file has been created

    return 0;
}

/**
 * Removes a dentry from the parent directory
 * @param p_inode
 * @param inode
 * @return
 */
// XXX errorhandling
int remove_dentry(const unsigned long p_inode, const uint64_t inode) {
//    auto f_path = bfs::path(ADAFS_DATA->dentry_path());
//    f_path /= to_string(p_inode);
//    if (!bfs::exists(f_path)) {
//        ADAFS_DATA->logger->error("remove_dentry() dentry_path '{}' not found", f_path.string());
//        return -ENOENT;
//    }
//
//    f_path /= inode;
//    // remove dentry here
//    bfs::remove(f_path);
//
//    // XXX make sure dentry has been deleted

    return 0;
}

/**
 * Checks if a directory has no dentries, i.e., is empty.
 * @param inode
 * @return bool
 */
bool is_dir_empty(const uint64_t inode) {
//    auto d_path = bfs::path(ADAFS_DATA->dentry_path());
//    // use hash function to path and append it to d_path
//    d_path /= to_string(ADAFS_DATA->hashf(inode.string()));
//
//    return bfs::is_empty(d_path);
    return false;
}
