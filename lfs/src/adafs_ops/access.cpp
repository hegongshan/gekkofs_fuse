//
// Created by evie on 3/28/17.
//

#include "access.h"
#include "metadata_ops.h"

/**
 * chk_access wrapper for opendir and open.
 * @param req
 * @param ino
 * @param flags from fuse_file_info
 * @return err
 */
int open_chk_access(fuse_req_t& req, fuse_ino_t ino, int flags) {
    // XXX error handling
    auto md = make_shared<Metadata>();

    auto err = get_metadata(*md, ino);

    if (err != 0) return err;

    int access = flags & O_ACCMODE; // flags & 3. 0 = R, 1 = W, 2 = RW

    ADAFS_DATA->spdlogger()->debug("access {} flags {}", access, flags);
    switch (access) {
        case O_RDONLY:
            return chk_access(req, *md, R_OK);
        case O_WRONLY:
            return chk_access(req, *md, W_OK);
        case O_RDWR:
            return chk_access(req, *md, R_OK | W_OK);
        default:
            return EACCES;
    }

}

/**
 * Checks access for mask (can be R_OK, W_OK, or X_OK (or combined) AFAIK and not verified) against metadata's mode.
 * First the mask is checked agains the 3 bits for the user, then for the 3 bits of the group, and lastly other.
 * If all three checks have failed, return EACCESS (no access)
 * @param req
 * @param md
 * @param mask
 * @return
 */
int chk_access(const fuse_req_t& req, const Metadata& md, int mask) {
    ADAFS_DATA->spdlogger()->debug("chk_access() enter: metadata_uid {} fusecontext_uid {} mask {}", md.uid(),
                                   fuse_req_ctx(req)->uid, mask);
    // root user is a god
    if (fuse_req_ctx(req)->uid == 0)
        return 0;

    //check user leftmost 3 bits for rwx in md->mode
    if (md.uid() == fuse_req_ctx(req)->uid) {
        // Because mode comes only with the first 3 bits used, the user bits have to be shifted to the right to compare
        if ((mask & md.mode() >> 6) == static_cast<unsigned int>(mask))
            return 0;
        else
            return EACCES;
    }

    //check group middle 3 bits for rwx in md->mode
    if (md.gid() == fuse_req_ctx(req)->gid) {
        if ((mask & md.mode() >> 3) == static_cast<unsigned int>(mask))
            return 0;
        else
            return EACCES;
    }

    //check other rightmost 3 bits for rwx in md->mode.
    // Because they are the rightmost bits they don't need to be shifted
    if ((mask & md.mode()) == static_cast<unsigned int>(mask)) {
        return 0;
    }

    return EACCES;
}

/**
 * Check if uid from fuse context (i.e., the caller) equals the uid from the object
 * @param req
 * @param md
 * @return
 */
int chk_uid(const fuse_req_t& req, const Metadata& md) {

    // root user is a god
    if (fuse_req_ctx(req)->uid == 0)
        return 0;

    // if user is the user of md, he/she has access
    if (fuse_req_ctx(req)->uid == md.uid())
        return 0;

    // else no permission
    return EPERM;
}

/**
 * Changes the mode from an object to a given mode. Permissions are NOT checked here
 * @param md
 * @param mode
 * @return
 */
// XXX error handling
int change_access(Metadata& md, mode_t mode, const bfs::path& path) {

//    auto path_hash = ADAFS_DATA->hashf(path.string());
//    md.mode((mode_t) mode);
//
//    write_metadata_field(md.mode(), path_hash, md_field_map.at(Md_fields::mode));
//
//#ifdef ACMtime
//    md.update_ACM_time(true, true, true);
//    write_metadata_field(md.atime(), path_hash, md_field_map.at(Md_fields::atime));
//    write_metadata_field(md.ctime(), path_hash, md_field_map.at(Md_fields::ctime));
//    write_metadata_field(md.mtime(), path_hash, md_field_map.at(Md_fields::mtime));
//#endif

    return 0;
}

/**
 * Changes the uid and gid from an object to a given mode. Only root can actually change gid and uid for now.
 * Normal users can't change the uid because they only have one.
 * And currently normal users can't change the group either.
 * @param md
 * @param uid
 * @param gid
 * @param path
 * @return
 */
int change_permissions(Metadata& md, uid_t uid, gid_t gid, const bfs::path& path) {
//    auto path_hash = ADAFS_DATA->hashf(path.string());

//    // XXX Users should be able to change the group to whatever groups they're belonging to. For now group can only
//    // XXX be changed to the active group they're belonging to.
//    if (fuse_get_context()->gid != gid)
//        return -EPERM;
//    // if nothing changed, nothing to do
//    if (md.uid() == uid && md.gid() == gid)
//        return 0;
//
//    // root can do anything
//    if (fuse_get_context()->uid == 0) {
//        md.uid(uid);
//        md.gid(gid);
//        write_metadata_field(md.gid(), path_hash, md_field_map.at(Md_fields::gid));
//        write_metadata_field(md.uid(), path_hash, md_field_map.at(Md_fields::uid));
//
//#ifdef ACMtime
//        md.update_ACM_time(true, true, true);
//        write_metadata_field(md.atime(), path_hash, md_field_map.at(Md_fields::atime));
//        write_metadata_field(md.ctime(), path_hash, md_field_map.at(Md_fields::ctime));
//        write_metadata_field(md.mtime(), path_hash, md_field_map.at(Md_fields::mtime));
//#endif
//        return 0;
//    }
    // if we get here, users what to change uid or gid to something else which is not permitted
    return EPERM;
}


