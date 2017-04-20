//
// Created by evie on 3/17/17.
//

#include "dentry_ops.h"

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
 * Creates an empty file in the dentry folder of the parent directory, acting as a dentry for lookup
 * @param parent_dir
 * @param inode
 * @return
 */
int create_dentry(const unsigned long p_inode, const uint64_t inode) {
//    ADAFS_DATA->logger->debug("create_dentry() enter with fname: {}", inode);
//    // XXX Errorhandling
//    auto f_path = bfs::path(ADAFS_DATA->dentry_path);
//    f_path /= to_string(p_inode);
//    if (!bfs::exists(f_path)) return -ENOENT;
//
//    f_path /= inode;
//
//    bfs::ofstream ofs{f_path};
//
//    // XXX make sure the file has been created

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
