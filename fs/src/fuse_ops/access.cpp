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

/** Change the permission bits of a file
 *
 * `fi` will always be NULL if the file is not currenly open, but
 * may also be NULL if the file is open.
 */
int adafs_chmod(const char* p, mode_t mode, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_chmod() enter: name '{}' mode {:o}", p, mode);
    auto path = bfs::path(p);
    auto md = make_shared<Metadata>();
    auto err = get_metadata(*md, path);

    if (err != 0) return err;

    // for change_access only the uid matters AFAIK
    err = chk_uid(*md);
    if (err != 0) return err;

    return change_access(*md, mode, path);
}

/** Change the owner and group of a file
 *
 * `fi` will always be NULL if the file is not currenly open, but
 * may also be NULL if the file is open.
 *
 * Unless FUSE_CAP_HANDLE_KILLPRIV is disabled, this method is
 * expected to reset the setuid and setgid bits.
 */
int adafs_chown(const char* p, uid_t uid, gid_t gid, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_chown() enter: name '{}' uid {} gid {}", p, uid, gid);
    auto path = bfs::path(p);
    auto md = make_shared<Metadata>();
    auto err = get_metadata(*md, path);

    if (err != 0) return err;

    // any ownership change requires the user of the object
    err = chk_uid(*md);
    if (err != 0) return err;

    return change_permissions(*md, uid, gid, path);
}
