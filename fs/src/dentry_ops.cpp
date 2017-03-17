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