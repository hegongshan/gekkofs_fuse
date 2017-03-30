//
// Created by evie on 3/28/17.
//

#include "access.h"

/**
 * Checks access for mask (can be R_OK, W_OK, or X_OK (or combined) AFAIK and not verified) against metadata's mode.
 * First the mask is checked agains the 3 bits for the user, then for the 3 bits of the group, and lastly other.
 * If all three checks have failed, return -EACCESS (no access)
 * @param md
 * @param mask
 * @return
 */
int chk_access(const Metadata& md, const int mask) {
    ADAFS_DATA->logger->debug("chk_access() enter: metadata_uid {} fusecontext_uid {}", md.uid(),
                              fuse_get_context()->uid);
    // root user is a god
    if (fuse_get_context()->uid == 0)
        return 0;

    //check user leftmost 3 bits for rwx in md->mode
    if (md.uid() == fuse_get_context()->uid) {
        // Because mode comes only with the first 3 bits used, the user bits have to be shifted to the right to compare
        if ((mask & md.mode() >> 6) == (unsigned int) mask)
            return 0;
        else
            return -EACCES;
    }

    //check group middle 3 bits for rwx in md->mode
    if (md.gid() == fuse_get_context()->gid) {
        if ((mask & md.mode() >> 3) == (unsigned int) mask)
            return 0;
        else
            return -EACCES;
    }

    //check other rightmost 3 bits for rwx in md->mode.
    // Because they are the rightmost bits they don't need to be shifted
    if ((mask & md.mode()) == (unsigned int) mask) {
        return 0;
    }

    return -EACCES;
}