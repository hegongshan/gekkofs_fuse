//
// Created by evie on 3/17/17.
//

#include "dentry_ops.h"

using namespace std;

/**
 * Called when a directory is created to init the corresponding dentry dir.
 * @param hash
 * @return
 */
bool init_dentry(const unsigned long& hash) {
    auto d_path = bfs::path(ADAFS_DATA->dentry_path);
    d_path /= to_string(hash);
    bfs::create_directories(d_path);

    return bfs::exists(d_path);
}

/**
 * Check if the file name can be found in the directory entries of parent_dir_hash
 * @param parent_dir_hash
 * @param fname
 * @return
 */
bool verify_dentry(const bfs::path& path) {
    auto d_path = bfs::path(ADAFS_DATA->dentry_path);
    if (path.has_parent_path()) { // non-root
        d_path /= to_string(ADAFS_DATA->hashf(path.parent_path().string()));
        d_path /= path.filename(); // root
    } else {
        d_path /= to_string(ADAFS_DATA->hashf(path.string()));
    }
    // if file path exists leaf name is a valid dentry of parent_dir
    return bfs::exists(d_path);
}


/**
 * Reads all directory entries in a directory with a given @hash. Returns 0 if successful.
 * @dir is assumed to be empty
 */
int read_dentries(vector<string>& dir, const unsigned long hash) {
    auto path = bfs::path(ADAFS_DATA->dentry_path);
    path /= to_string(hash);
    if (!bfs::exists(path)) return 1;
    // shortcut if path is empty = no files in directory
    if (bfs::is_empty(path)) return 0;

    // Below can be simplified with a C++11 range based loop? But how? :( XXX
    bfs::directory_iterator end_dir_it;
    for (bfs::directory_iterator dir_it(path); dir_it != end_dir_it; ++dir_it) {
        const bfs::path cp = (*dir_it);
        dir.push_back(cp.filename().string());
    }
    return 0;
}

/**
 * Creates an empty file in the dentry folder of the parent directory, acting as a dentry for lookup
 * @param parent_dir
 * @param fname
 * @return
 */
int create_dentry(const unsigned long parent_dir_hash, const string& fname) {
    ADAFS_DATA->logger->debug("create_dentry() enter with fname: {}", fname);
    // XXX Errorhandling
    auto f_path = bfs::path(ADAFS_DATA->dentry_path);
    f_path /= to_string(parent_dir_hash);
    if (!bfs::exists(f_path)) return -ENOENT;

    f_path /= fname;

    bfs::ofstream ofs{f_path};

    // XXX make sure the file has been created

    return 0;
}

int remove_dentry(const unsigned long parent_dir_hash, const string& fname) {
    auto f_path = bfs::path(ADAFS_DATA->dentry_path);
    f_path /= to_string(parent_dir_hash);
    if (!bfs::exists(f_path)) {
        ADAFS_DATA->logger->error("remove_dentry() dentry_path '{}' not found", f_path.string());
        return -ENOENT;
    }

    f_path /= fname;
    // remove dentry here
    bfs::remove(f_path);

    // XXX make sure dentry has been deleted

    return 0;
}