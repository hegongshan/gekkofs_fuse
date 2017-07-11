//
// Created by draze on 3/5/17.
//

#include "mdata_ops.hpp"
#include "dentry_ops.hpp"
#include "../rpc/client/c_dentry.hpp"
#include "../rpc/client/c_metadata.hpp"

using namespace std;

int write_all_metadata(const Metadata& md) {
    auto inode_key = fmt::FormatInt(md.inode_no()).str();
    // TODO this should be somewhat a batch operation or similar. this errorhandling is bs
    if (!db_put_mdata(db_build_mdata_key(
            inode_key, std::get<to_underlying(Md_fields::atime)>(md_field_map)), md.atime()))
        return EIO;
    if (!db_put_mdata(db_build_mdata_key(
            inode_key, std::get<to_underlying(Md_fields::mtime)>(md_field_map)), md.mtime()))
        return EIO;
    if (!db_put_mdata(db_build_mdata_key(
            inode_key, std::get<to_underlying(Md_fields::ctime)>(md_field_map)), md.ctime()))
        return EIO;
    if (!db_put_mdata(db_build_mdata_key(
            inode_key, std::get<to_underlying(Md_fields::uid)>(md_field_map)), md.uid()))
        return EIO;
    if (!db_put_mdata(db_build_mdata_key(
            inode_key, std::get<to_underlying(Md_fields::gid)>(md_field_map)), md.gid()))
        return EIO;
    if (!db_put_mdata(db_build_mdata_key(
            inode_key, std::get<to_underlying(Md_fields::mode)>(md_field_map)), md.mode()))
        return EIO;
    if (!db_put_mdata(db_build_mdata_key(
            inode_key, std::get<to_underlying(Md_fields::inode_no)>(md_field_map)), md.inode_no()))
        return EIO;
    if (!db_put_mdata(db_build_mdata_key(
            inode_key, std::get<to_underlying(Md_fields::link_count)>(md_field_map)), md.link_count()))
        return EIO;
    if (!db_put_mdata(db_build_mdata_key(
            inode_key, std::get<to_underlying(Md_fields::size)>(md_field_map)), md.size()))
        return EIO;
    if (!db_put_mdata(db_build_mdata_key(
            inode_key, std::get<to_underlying(Md_fields::blocks)>(md_field_map)), md.blocks()))
        return EIO;

    return 0;
}

// TODO error handling.
int read_all_metadata(Metadata& md, const fuse_ino_t inode) {
    auto inode_key = fmt::FormatInt(inode).str();

    md.atime(db_get_mdata<time_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::atime)>(md_field_map))));
    md.mtime(db_get_mdata<time_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::mtime)>(md_field_map))));
    md.ctime(db_get_mdata<time_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::ctime)>(md_field_map))));
    md.uid(db_get_mdata<uid_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::uid)>(md_field_map))));
    md.gid(db_get_mdata<gid_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::gid)>(md_field_map))));
    md.mode(db_get_mdata<mode_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::mode)>(md_field_map))));
    md.inode_no(db_get_mdata<fuse_ino_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::inode_no)>(md_field_map))));
    md.link_count(db_get_mdata<nlink_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::link_count)>(md_field_map))));
    md.size(db_get_mdata<off_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::size)>(md_field_map))));
    md.blocks(db_get_mdata<blkcnt_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::blocks)>(md_field_map))));
    return 0;
}

/**
 * Removes the metadata of a file based on the inode. The function does not check if the inode exists. This should
 * be done by the get_metadata() (implicit or explicit)
 * @param inode
 * @return err
 */
int remove_all_metadata(const fuse_ino_t inode) {
    auto inode_key = fmt::FormatInt(inode).str();
    // TODO this should be somewhat a batch operation or similar. this errorhandling is bs
    if (!db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::atime)>(md_field_map))))
        return EIO;
    if (!db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::mtime)>(md_field_map))))
        return EIO;
    if (!db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::ctime)>(md_field_map))))
        return EIO;
    if (!db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::uid)>(md_field_map))))
        return EIO;
    if (!db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::gid)>(md_field_map))))
        return EIO;
    if (!db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::mode)>(md_field_map))))
        return EIO;
    if (!db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::inode_no)>(md_field_map))))
        return EIO;
    if (!db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::link_count)>(md_field_map))))
        return EIO;
    if (!db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::size)>(md_field_map))))
        return EIO;
    if (!db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::blocks)>(md_field_map))))
        return EIO;
    return 0;
}

/**
 * Gets the metadata via its inode and puts it into an Metadata object.
 * @param md
 * @param inode
 * @return err
 */
int get_metadata(Metadata& md, const fuse_ino_t inode) {
    ADAFS_DATA->spdlogger()->debug("get_metadata() enter for inode {}", inode);
    // Verify that the file's inode exists
    if (db_mdata_exists(inode)) {
        read_all_metadata(md, inode);
        return 0;
    } else
        return ENOENT;
}

/**
 * Gets the metadata via its inode and puts it into the stat struct.
 *
 * @param req
 * @param attr
 * @param inode
 * @return err
 */
int get_attr(struct stat& attr, const fuse_ino_t inode) {

    // XXX look in cache first
    auto md = make_shared<Metadata>();
    auto err = get_metadata(*md, inode);

    metadata_to_stat(*md, attr);

    return err;
}

void metadata_to_stat(const Metadata& md, struct stat& attr) {
    attr.st_ino = md.inode_no();
    attr.st_mode = md.mode();
    attr.st_nlink = md.link_count();
    attr.st_uid = md.uid();
    attr.st_gid = md.gid();
    attr.st_size = md.size();
    attr.st_blksize = ADAFS_DATA->blocksize();
    attr.st_blocks = md.blocks();
    attr.st_atim.tv_sec = md.atime();
    attr.st_mtim.tv_sec = md.mtime();
    attr.st_ctim.tv_sec = md.ctime();
}

/**
 * Initializes the metadata for the given parameters. Return value not returning the state yet.
 * Function will additionally return filled fuse_entry_param
 * @param inode
 * @param uid
 * @param gid
 * @param mode
 * @return always 0
 */
int init_metadata_fep(struct fuse_entry_param& fep, const fuse_ino_t inode, const uid_t uid, const gid_t gid,
                      mode_t mode) {
    Metadata md{mode, uid, gid, inode};
    if ((mode & S_IFDIR) == S_IFDIR) {
        // XXX just visual. size computation of directory should be done properly at some point
        md.size(ADAFS_DATA->blocksize());
    }
    write_all_metadata(md);

    // create dentry for Linux
    fep.entry_timeout = 1.0;
    fep.attr_timeout = 1.0;
    fep.ino = md.inode_no();
    //fill fep.attr with the metadata information
    metadata_to_stat(md, fep.attr);
    return 0;
}

/**
 * Initializes the metadata for the given parameters. Return value not returning the state yet.
 * @param inode
 * @param uid
 * @param gid
 * @param mode
 * @return always 0
 */
int init_metadata(const fuse_ino_t inode, const uid_t uid, const gid_t gid, mode_t mode) {
    Metadata md{mode, uid, gid, inode};
    if ((mode & S_IFDIR) == S_IFDIR) {
        // XXX just visual. size computation of directory should be done properly at some point
        md.size(ADAFS_DATA->blocksize());
    }
    write_all_metadata(md);

    return 0;
}

/**
 * Creates a new node (file or directory) in the file system. Fills given fuse_entry_param.
 * @param req
 * @param fep
 * @param parent
 * @param name
 * @param mode
 * @return err
 */
int create_node(struct fuse_entry_param& fep, fuse_ino_t parent, const char* name, const uid_t uid, const gid_t gid,
                mode_t mode) {
//    // create inode number
//    auto new_inode = Util::generate_inode_no();
//    // create dentry
//    create_dentry(parent, new_inode, name, mode);
//    // create metadata and fill fuse entry param
//    init_metadata_fep(fep, new_inode, uid, gid, mode);

    int err;
    // create new inode number
    fuse_ino_t new_inode = Util::generate_inode_no();
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(RPC_DATA->get_dentry_hashable(parent, name));
        if (ADAFS_DATA->is_local_op(recipient)) { // local dentry create
            err = create_dentry(parent, new_inode, name, mode);
        } else { // remote dentry create
            err = rpc_send_create_dentry(recipient, parent, name, mode, new_inode);
        }
        if (err != 0) { // failure in dentry creation
            ADAFS_DATA->spdlogger()->error("Failed to create a dentry");
            return err;
        }
        // calculate recipient again for new inode because it could hash somewhere else
        recipient = RPC_DATA->get_rpc_node(fmt::FormatInt(new_inode).str());
        if (ADAFS_DATA->is_local_op(recipient)) { // local metadata init
            err = init_metadata_fep(fep, new_inode, uid, gid, mode);
        } else { // remote metadata init
            err = rpc_send_create_mdata(recipient, uid, gid, mode, new_inode);
            if (err == 0) {
                // Because we don't want to return the metadata init values through the RPC
                // we just set dummy values here with the most important bits
                fep.ino = new_inode;
                fep.attr.st_ino = new_inode;
                fep.attr.st_mode = mode;
                fep.attr.st_blocks = 0;
                fep.attr.st_gid = gid;
                fep.attr.st_uid = uid;
                fep.attr.st_nlink = 0;
                fep.attr.st_size = 0;
                fep.entry_timeout = 1.0;
                fep.attr_timeout = 1.0;
            } else {
                // TODO remove created dentry
            }
        }
    } else { //local single node operation
        // XXX check permissions (omittable), should create node be atomic?
        // create dentry
        err = create_dentry(parent, new_inode, name, mode);
        if (err != 0) { // failure in dentry creation
            ADAFS_DATA->spdlogger()->error("Failed to create a dentry");
            return err;
        }
        // create metadata and fill fuse entry param
        err = init_metadata_fep(fep, new_inode, uid, gid, mode);
    }
    if (err != 0)
        ADAFS_DATA->spdlogger()->error("Failed to create metadata");
    // TODO remove created dentry

    return err;
}

int remove_node(fuse_ino_t parent, const char* name) {

    fuse_ino_t del_inode;
    int err;

    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(RPC_DATA->get_dentry_hashable(parent, name));
        if (ADAFS_DATA->is_local_op(recipient)) { // local dentry removal
            // Remove denty returns <err, inode_of_dentry> pair
            tie(err, del_inode) = remove_dentry(parent, name);
        } else { // remote dentry removal
            err = rpc_send_remove_dentry(recipient, parent, name, del_inode);
        }
        if (err != 0) {
            ADAFS_DATA->spdlogger()->error("Failed to remove dentry");
            return err;
        }
        // recalculate recipient for metadata removal
        recipient = RPC_DATA->get_rpc_node(fmt::FormatInt(del_inode).str());
        if (ADAFS_DATA->is_local_op(recipient)) { // local metadata removal
            err = remove_all_metadata(del_inode);
        } else { // remote metadata removal
            err = rpc_send_remove_mdata(recipient, del_inode);
        }
    } else { // single node local operation
        // Remove denty returns <err, inode_of_dentry> pair
        tie(err, del_inode) = remove_dentry(parent, name);
        if (err != 0) {
            ADAFS_DATA->spdlogger()->error("Failed to remove dentry");
            return err;
        }
        // Remove inode
        err = remove_all_metadata(del_inode);
    }

    if (err != 0)
        ADAFS_DATA->spdlogger()->error("Failed to remove metadata");

    /* TODO really consider if this is even required in a distributed setup, I'd argue: No
     * XXX consider the whole lookup count functionality. We need something like a hashtable here, which marks the file
     * for removal. If forget is then called, the file should be really removed. (see forget comments)
     * Any fuse comments that increment the lookup count will show the file as deleted after unlink and before/after forget.
     * symlinks, hardlinks, devices, pipes, etc all work differently with forget and unlink
     */

    return err;
}


