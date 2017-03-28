//
// Created by evie on 3/28/17.
//

#include "../main.h"
#include "../fuse_ops.h"

/** Get file system statistics
	 *
	 * The 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
	 */
int adafs_statfs(const char* p, struct statvfs* statvfs) {
    ADAFS_DATA->logger->info("##### FUSE FUNC ###### adafs_statfs() enter: name '{}'", p);
    // XXX to be implemented for rm -rf
    return 0;
}