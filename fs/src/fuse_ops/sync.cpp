//
// Created by draze on 3/23/17.
//

#include "../main.h"
#include "../fuse_ops.h"

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().	This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.	It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 */
// currently a NO-OP in ADA-FS
int adafs_flush(const char* p, struct fuse_file_info*) {
    ADAFS_DATA->logger->debug(" ##### FUSE FUNC ###### adafs_flush() enter: name '{}'", p);
    return 0;
}
