//
// Created by draze on 3/19/17.
//

#include "../fuse_utils.h"

using namespace std;

int chk_access(const Metadata& md, const int mode) {
    ADAFS_DATA->logger->info("chk_access() enter: md.uid: {}, fusecontextuid: {}", md.uid(), fuse_get_context()->uid);
    // root user is a god
    if (fuse_get_context()->uid == 0)
        return 0;

    // mode should be unsigned int. Init here.

    //check user
    if (md.uid() == fuse_get_context()->uid) {
        ADAFS_DATA->logger->info("Metadata UID: {}, fuse context uid: {}, mode: {}",
                                 md.uid(), fuse_get_context()->uid, mode);
        // XXX
    }

    //check group
    if (md.gid() == fuse_get_context()->gid) {
        ADAFS_DATA->logger->info("Metadata GID: {}, fuse context gid: {}, mode: {}",
                                 md.uid(), fuse_get_context()->gid, mode);
        // XXX
    }
    //check public
    ADAFS_DATA->logger->info("mode {}, mdi mode: {}, mdi & mode {}", mode, (int) md.mode(), (mode & md.mode()));

    if ((mode & (int) md.mode()) == mode) {
        return 0;
    }

    return -EACCES;
}
