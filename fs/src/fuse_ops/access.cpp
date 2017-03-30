//
// Created by draze on 3/19/17.
//

#include "../main.h"
#include "../fuse_ops.h"
#include "../adafs_ops/metadata_ops.h"
#include "../adafs_ops/access.h"

using namespace std;

/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 * This method is not called under Linux kernel versions 2.4.x
 */
int adafs_access(const char* p, int mask) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_access() enter: name '{}' mask {}", p, mask);
    auto path = bfs::path(p);

    auto md = make_shared<Metadata>();
    // XXX error handling
    auto err = get_metadata(*md, path);
    if (err != 0) return err;

    return chk_access(*md, mask);
}

