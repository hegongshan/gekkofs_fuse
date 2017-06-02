//
// Created by draze on 3/5/17.
//

#include "mdata_ops.hpp"
#include "dentry_ops.hpp"



// TODO error handling. Each read_metadata_field should check for boolean, i.e., if I/O failed.
bool write_all_metadata(const Metadata& md, const fuse_ino_t inode) {
    write_metadata_field(md.atime(), std::get<to_underlying(Md_fields::atime)>(md_field_map), inode);
    write_metadata_field(md.mtime(), std::get<to_underlying(Md_fields::mtime)>(md_field_map), inode);
    write_metadata_field(md.ctime(), std::get<to_underlying(Md_fields::ctime)>(md_field_map), inode);
    write_metadata_field(md.uid(), std::get<to_underlying(Md_fields::uid)>(md_field_map), inode);
    write_metadata_field(md.gid(), std::get<to_underlying(Md_fields::gid)>(md_field_map), inode);
    write_metadata_field(md.mode(), std::get<to_underlying(Md_fields::mode)>(md_field_map), inode);
    write_metadata_field(md.inode_no(), std::get<to_underlying(Md_fields::inode_no)>(md_field_map), inode);
    write_metadata_field(md.link_count(), std::get<to_underlying(Md_fields::link_count)>(md_field_map), inode);
    write_metadata_field(md.size(), std::get<to_underlying(Md_fields::size)>(md_field_map), inode);
    write_metadata_field(md.blocks(), std::get<to_underlying(Md_fields::blocks)>(md_field_map), inode);

    return true;
}

// TODO error handling. Each read_metadata_field should check for nullptr, i.e., if I/O failed.
bool read_all_metadata(Metadata& md, const fuse_ino_t inode) {
    md.atime(*read_metadata_field<time_t>(std::get<to_underlying(Md_fields::atime)>(md_field_map), inode));
    md.mtime(*read_metadata_field<time_t>(std::get<to_underlying(Md_fields::mtime)>(md_field_map), inode));
    md.ctime(*read_metadata_field<time_t>(std::get<to_underlying(Md_fields::ctime)>(md_field_map), inode));
    md.uid(*read_metadata_field<uid_t>(std::get<to_underlying(Md_fields::uid)>(md_field_map), inode));
    md.gid(*read_metadata_field<gid_t>(std::get<to_underlying(Md_fields::gid)>(md_field_map), inode));
    md.mode(*read_metadata_field<mode_t>(std::get<to_underlying(Md_fields::mode)>(md_field_map), inode));
    md.inode_no(*read_metadata_field<fuse_ino_t>(std::get<to_underlying(Md_fields::inode_no)>(md_field_map), inode));
    md.link_count(*read_metadata_field<nlink_t>(std::get<to_underlying(Md_fields::link_count)>(md_field_map), inode));
    md.size(*read_metadata_field<off_t>(std::get<to_underlying(Md_fields::size)>(md_field_map), inode));
    md.blocks(*read_metadata_field<blkcnt_t>(std::get<to_underlying(Md_fields::blocks)>(md_field_map), inode));
    return true;
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
    auto path = bfs::path(ADAFS_DATA->inode_path());
    path /= fmt::FormatInt(inode).c_str();
    if (bfs::exists(path)) {
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
 * Removes the metadata of a file based on the inode. The function does not check if the inode path exists. This should
 * be done by the get_metadata() (implicit or explicit)
 * @param inode
 * @return err
 */
int remove_metadata(const fuse_ino_t inode) {
    // XXX Errorhandling
    auto i_path = bfs::path(ADAFS_DATA->inode_path());
    i_path /= fmt::FormatInt(inode).c_str();

    bfs::remove_all(i_path);
    // XXX make sure metadata has been deleted

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




