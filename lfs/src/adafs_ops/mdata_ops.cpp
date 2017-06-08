//
// Created by draze on 3/5/17.
//

#include "mdata_ops.hpp"
#include "dentry_ops.hpp"

using namespace std;

// TODO error handling.
int write_all_metadata(const Metadata& md, const fuse_ino_t inode) {
    auto inode_key = fmt::FormatInt(inode).str();

    db_put_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::atime)>(md_field_map)), md.atime());
    db_put_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::mtime)>(md_field_map)), md.mtime());
    db_put_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::ctime)>(md_field_map)), md.ctime());
    db_put_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::uid)>(md_field_map)), md.uid());
    db_put_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::gid)>(md_field_map)), md.gid());
    db_put_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::mode)>(md_field_map)), md.mode());
    db_put_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::inode_no)>(md_field_map)),
                 md.inode_no());
    db_put_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::link_count)>(md_field_map)),
                 md.link_count());
    db_put_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::size)>(md_field_map)), md.size());
    db_put_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::blocks)>(md_field_map)), md.blocks());
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
    md.uid(db_get_mdata<uid_t>(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::uid)>(md_field_map))));
    md.gid(db_get_mdata<gid_t>(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::gid)>(md_field_map))));
    md.mode(db_get_mdata<mode_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::mode)>(md_field_map))));
    md.inode_no(db_get_mdata<fuse_ino_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::inode_no)>(md_field_map))));
    md.link_count(db_get_mdata<nlink_t>(
            db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::link_count)>(md_field_map))));
    md.size(db_get_mdata<off_t>(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::size)>(md_field_map))));
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
    // TODO error handling
    auto inode_key = fmt::FormatInt(inode).str();
    db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::atime)>(md_field_map)));
    db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::mtime)>(md_field_map)));
    db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::ctime)>(md_field_map)));
    db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::uid)>(md_field_map)));
    db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::gid)>(md_field_map)));
    db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::mode)>(md_field_map)));
    db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::inode_no)>(md_field_map)));
    db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::link_count)>(md_field_map)));
    db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::size)>(md_field_map)));
    db_delete_mdata(db_build_mdata_key(inode_key, std::get<to_underlying(Md_fields::blocks)>(md_field_map)));
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
 * Creates a new node (file or directory) in the file system. Fills given fuse_entry_param.
 * @param req
 * @param fep
 * @param parent
 * @param name
 * @param mode
 * @return err
 */
int create_node(fuse_req_t& req, struct fuse_entry_param& fep, fuse_ino_t parent, const string& name, mode_t mode) {
    // create metadata of new file (this will also create a new inode number)
    // mode is used here to init metadata
    auto md = make_shared<Metadata>(mode, fuse_req_ctx(req)->uid, fuse_req_ctx(req)->gid, req);
    if ((mode & S_IFDIR) == S_IFDIR) {
        md->size(
                ADAFS_DATA->blocksize()); // XXX just visual. size computation of directory should be done properly at some point
    }
    // create directory entry (can fail) in adafs
    create_dentry(parent, md->inode_no(), name, mode);

    // write metadata
    write_all_metadata(*md, md->inode_no());

    // create dentry for Linux
    fep.entry_timeout = 1.0;
    fep.attr_timeout = 1.0;
    fep.ino = md->inode_no();
    //fill fep->attr with the metadata information
    metadata_to_stat(*md, fep.attr);

    return 0;
}




