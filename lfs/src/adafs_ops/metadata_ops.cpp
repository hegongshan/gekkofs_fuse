//
// Created by draze on 3/5/17.
//

#include "metadata_ops.h"
#include "dentry_ops.h"



// TODO error handling. Each read_metadata_field should check for boolean, i.e., if I/O failed.
bool write_all_metadata(const Metadata& md, const uint64_t inode) {
    write_metadata_field(md.atime(), md_field_map.at(Md_fields::atime), inode);
    write_metadata_field(md.mtime(), md_field_map.at(Md_fields::mtime), inode);
    write_metadata_field(md.ctime(), md_field_map.at(Md_fields::ctime), inode);
    write_metadata_field(md.uid(), md_field_map.at(Md_fields::uid), inode);
    write_metadata_field(md.gid(), md_field_map.at(Md_fields::gid), inode);
    write_metadata_field(md.mode(), md_field_map.at(Md_fields::mode), inode);
    write_metadata_field(md.inode_no(), md_field_map.at(Md_fields::inode_no), inode);
    write_metadata_field(md.link_count(), md_field_map.at(Md_fields::link_count), inode);
    write_metadata_field(md.size(), md_field_map.at(Md_fields::size), inode);
    write_metadata_field(md.blocks(), md_field_map.at(Md_fields::blocks), inode);

    return true;
}

// TODO error handling. Each read_metadata_field should check for nullptr, i.e., if I/O failed.
bool read_all_metadata(Metadata& md, const uint64_t inode) {
    md.atime(*read_metadata_field<time_t>(md_field_map.at(Md_fields::atime), inode));
    md.mtime(*read_metadata_field<time_t>(md_field_map.at(Md_fields::mtime), inode));
    md.ctime(*read_metadata_field<time_t>(md_field_map.at(Md_fields::ctime), inode));
    md.uid(*read_metadata_field<uint32_t>(md_field_map.at(Md_fields::uid), inode));
    md.gid(*read_metadata_field<uint32_t>(md_field_map.at(Md_fields::gid), inode));
    md.mode(*read_metadata_field<uint32_t>(md_field_map.at(Md_fields::mode), inode));
    md.inode_no(*read_metadata_field<uint64_t>(md_field_map.at(Md_fields::inode_no), inode));
    md.link_count(*read_metadata_field<uint32_t>(md_field_map.at(Md_fields::link_count), inode));
    md.size(*read_metadata_field<uint32_t>(md_field_map.at(Md_fields::size), inode));
    md.blocks(*read_metadata_field<uint32_t>(md_field_map.at(Md_fields::blocks), inode));
    return true;
}

int get_metadata(Metadata& md, const uint64_t inode) {
    ADAFS_DATA->spdlogger()->debug("get_metadata() enter for inode {}", inode);
    // Verify that the file is a valid dentry of the parent dir XXX put back in later when we have dentry ops
//    if (verify_dentry(inode)) {
//        // Metadata for file exists
//        read_all_metadata(req, md, ADAFS_DATA->hashf(inode));
//        return 0;
//    } else {
//        return -ENOENT;
//    }
    auto path = bfs::path(ADAFS_DATA->inode_path());
    path /= to_string(inode);
    if (bfs::exists(path)) {
        read_all_metadata(md, inode);
        return 0;
    } else
        return ENOENT;
}

/**
 * Returns the metadata of an object based on its hash
 * @param path
 * @return
 */
// XXX Errorhandling
//int remove_metadata(const unsigned long hash) {
//    auto i_path = bfs::path(ADAFS_DATA->inode_path);
//    i_path /= to_string(hash);
//    // XXX below could be omitted
//    if (!bfs::exists(i_path)) {
//        ADAFS_DATA->spdlogger->error("remove_metadata() metadata_path '{}' not found", i_path.string());
//        return -ENOENT;
//    }
//
//    bfs::remove_all(i_path);
//    // XXX make sure metadata has been deleted
//
//    return 0;
//}






