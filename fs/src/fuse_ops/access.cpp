//
// Created by draze on 3/19/17.
//

#include "../main.h"
#include "../fuse_ops.h"

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
    ADAFS_DATA->logger->info("##### FUSE FUNC ###### adafs_access() enter: name '{}' mask {}", p, mask);
    // XXX To be implemented for rm
    return 0;
}

