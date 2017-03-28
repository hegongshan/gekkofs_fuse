//
// Created by evie on 3/28/17.
//

#include "access.h"

/**
 *
 * @param md
 * @param mode
 * @return
 */
int chk_access(const Metadata& md, const int mode) {
    ADAFS_DATA->logger->debug("chk_access() enter: metadata_uid {} fusecontext_uid {}", md.uid(),
                              fuse_get_context()->uid);
    // root user is a god
    if (fuse_get_context()->uid == 0)
        return 0;

    //check user leftmost 3 bits for rwx in md->mode
    if (md.uid() == fuse_get_context()->uid) {
        // Because mode comes only with the first 3 bits used, the user bits have to be shifted to the right to compare
        if ((mode & md.mode() >> 6) == (unsigned int) mode)
            return 0;
        else
            return -EACCES;
    }

    //check group middle 3 bits for rwx in md->mode
    if (md.gid() == fuse_get_context()->gid) {
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