//
// Created by draze on 3/19/17.
//

#include "../fuse_utils.h"

using namespace std;

int chk_access(const Metadata& md, const int mode) {
    ADAFS_DATA->logger->debug("chk_access() enter: md.uid: {}, fusecontextuid: {}", md.uid(), fuse_get_context()->uid);
    // root user is a god
    if (fuse_get_context()->uid == 0)
        return 0;

    //check user leftmost 3 bits for rwx in md->mode
    if (md.uid() == fuse_get_context()->uid) {
        ADAFS_DATA->logger->debug("Metadata UID: {}, fuse context uid: {}, mode: {}",
                                  md.uid(), fuse_get_context()->uid, mode);
        // Because mode comes only with the first 3 bits used, the user bits have to be shifted to the right to compare
        if ((mode & md.mode() >> 6) == (unsigned int) mode)
            return 0;
        else
            return -EACCES;
    }

    //check group middle 3 bits for rwx in md->mode
    if (md.gid() == fuse_get_context()->gid) {
        ADAFS_DATA->logger->debug("Metadata GID: {}, fuse context gid: {}, mode: {}",
                                  md.uid(), fuse_get_context()->gid, mode);
        if ((mode & md.mode() >> 3) == (unsigned int) mode)
            return 0;
        else
            return -EACCES;
    }

    //check other rightmost 3 bits for rwx in md->mode.
    // Because they are the rightmost bits they don't need to be shifted
    if ((mode & md.mode()) == (unsigned int) mode) {
        return 0;
    }

    return -EACCES;
}
