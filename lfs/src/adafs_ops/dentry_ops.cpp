//
// Created by evie on 3/17/17.
//

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include "dentry_ops.hpp"
#include "db_ops.hpp"

#include "../classes/dentry.h"

using namespace std;

/**
 * Initializes the dentry directory to hold future dentries
 * @param inode
 * @return err
 */
bool init_dentry_dir(const fuse_ino_t inode) {
    auto d_path = bfs::path(ADAFS_DATA->dentry_path());
    d_path /= fmt::FormatInt(inode).c_str();
    bfs::create_directories(d_path);
    // XXX This might not be needed as it is another access to the underlying file system
//    return bfs::exists(d_path);
    return 0;
}

/**
 * Destroys the dentry directory
 * @param inode
 * @return 0 if successfully deleted
 */
int destroy_dentry_dir(const fuse_ino_t inode) {
    auto d_path = bfs::path(ADAFS_DATA->dentry_path());
    d_path /= fmt::FormatInt(inode).c_str();

    // remove dentry dir
    bfs::remove_all(d_path);

    return 0;
}

/**
 * Check if the file name can be found in the directory entries of parent_dir_hash
 * @param parent_dir_hash
 * @param fname
 * @return
 */
bool verify_dentry(const fuse_ino_t inode) {
    // XXX do I need this?
    return false;
//    auto d_path = bfs::path(ADAFS_DATA->dentry_path());
//    if (inode != ADAFS_ROOT_INODE) { // non-root
//        d_path /=
//    }
//
//
//    if (inode.has_parent_path()) { // non-root
//        d_path /= fmt::FormatInt(ADAFS_DATA->hashf(inode.parent_path().string()));
//        d_path /= inode.filename(); // root
//    } else {
//        d_path /= fmt::FormatInt(ADAFS_DATA->hashf(inode.string()));
//    }
//    // if file path exists leaf name is a valid dentry of parent_dir
//    return bfs::exists(d_path);
}


/**
 * Reads all directory entries in a directory with a given @hash. Returns 0 if successful.
 * @dir is assumed to be empty
 */
int read_dentries(const fuse_ino_t p_inode, const fuse_ino_t inode) {
//    auto path = bfs::path(ADAFS_DATA->dentry_path());
//    path /= fmt::FormatInt(inode);
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
 */
void get_dentries(vector<Dentry>& dentries, const fuse_ino_t dir_inode) {
    db_get_dentries(dentries, dir_inode);
}


/**
 * Gets the inode of a directory entry
 * @param req
 * @param parent_inode
 * @param name
 * @return pair<err, inode>
 */
pair<int, fuse_ino_t> do_lookup(fuse_req_t& req, const fuse_ino_t p_inode, const string& name) {
    string val; // will we filled by dentry exist check
    if (db_dentry_exists(p_inode, name, val) == 0) { // dentry NOT found
        return make_pair(ENOENT, INVALID_INODE);
    }

    auto pos = val.find("_");
    auto inode = static_cast<fuse_ino_t>(stoul(val.substr(0, pos)));

    return make_pair(0, inode);
}


/**
 * Creates an empty file in the dentry folder of the parent directory, acting as a dentry for lookup
 * @param parent_dir
 * @param name
 * @return
 */
int create_dentry(const fuse_ino_t p_inode, const fuse_ino_t inode, const string& name, mode_t mode) {

    auto key = "d_"s + fmt::FormatInt(p_inode).str() + "_"s + name;
    auto val = fmt::FormatInt(inode).str() + "_"s + fmt::FormatInt(mode).str();
    // XXX check later if we need to check if dentry of father already exists
    return db_put_dentry(key, val);
}

/**
 * Removes a dentry from the parent directory. It is not tested if the parent inode path exists. This should have been
 * done by do_lookup preceeding this call (impicit or explicit).
 * @param p_inode
 * @param name
 * @return pair<err, inode>
 */
// XXX errorhandling
pair<int, fuse_ino_t> remove_dentry(const fuse_ino_t p_inode, const string &name) {
    int inode; // file inode to be read from dentry before deletion
    auto d_path = bfs::path(ADAFS_DATA->dentry_path());
    d_path /= fmt::FormatInt(p_inode).c_str();
    d_path /= name;

    // retrieve inode number of dentry
    bfs::ifstream ifs{d_path};
    boost::archive::binary_iarchive ba(ifs);
    ba >> inode;

    bfs::remove(d_path);

    // XXX make sure dentry has been deleted

    return make_pair(0, inode);
}

/**
 * Checks if a directory has no dentries, i.e., is empty. Returns zero if empty, or err code.
 * @param inode
 * @return err
 */
int is_dir_empty(const fuse_ino_t inode) {
    auto d_path = bfs::path(ADAFS_DATA->dentry_path());
    d_path /= fmt::FormatInt(inode).c_str();
    if (bfs::is_empty(d_path))
        return 0;
    else
        return ENOTEMPTY;

}
