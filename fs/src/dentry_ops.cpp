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
    auto path = bfs::path(ADAFS_DATA->dentry_path);
    path.append(to_string(hash));
    bfs::create_directories(path);

    return bfs::exists(path);
}
