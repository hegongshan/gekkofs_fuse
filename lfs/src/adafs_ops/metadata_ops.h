//
// Created by draze on 3/5/17.
//

#ifndef FS_METADATA_OPS_H
#define FS_METADATA_OPS_H

#include "../main.h"
#include "../classes/metadata.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

using namespace std;

// mapping of enum to string to get the file names for metadata
enum class Md_fields { atime, mtime, ctime, uid, gid, mode, inode_no, link_count, size, blocks };

// TODO make static so that it is resolved at compile time
const std::map<Md_fields, std::string> md_field_map = {
        {Md_fields::atime,      "/atime"},
        {Md_fields::mtime,      "/mtime"},
        {Md_fields::ctime,      "/ctime"},
        {Md_fields::uid,        "/uid"},
        {Md_fields::gid,        "/gid"},
        {Md_fields::mode,       "/mode"},
        {Md_fields::inode_no,   "/inode_no"},
        {Md_fields::link_count, "/link_count"},
        {Md_fields::size,       "/size"},
        {Md_fields::blocks,     "/blocks"}
};

bool write_all_metadata(const Metadata& md, const uint64_t inode);

// TODO error handling.
template<typename T>
bool write_metadata_field(const T& field, const string& field_name, const uint64_t inode) {
    auto i_path = bfs::path(ADAFS_DATA->inode_path());
    i_path /= to_string(inode);
    bfs::create_directories(i_path);
    i_path /= field_name;

    bfs::ofstream ofs{i_path};
    // write to disk in binary form
    boost::archive::binary_oarchive ba(ofs);
    ba << field;

    return true;
}

bool read_all_metadata(Metadata& md, const uint64_t inode);

// TODO error handling
template<typename T>
unique_ptr<T> read_metadata_field(const string& field_name, const uint64_t inode) {
    auto path = bfs::path(ADAFS_DATA->inode_path());
    path /= to_string(inode);
    path /= field_name;
    if (!bfs::exists(path)) return nullptr;

    bfs::ifstream ifs{path};
    //fast error checking
    //ifs.good()
    boost::archive::binary_iarchive ba(ifs);
    auto field = make_unique<T>();
    ba >> *field;
    return field;
}

int get_metadata(Metadata& md, const uint64_t inode);

int get_attr(struct stat& attr, const uint64_t inode);

void metadata_to_stat(const Metadata& md, struct stat& attr);

//int remove_metadata(const unsigned long hash);

int create_node(fuse_req_t& req, struct fuse_entry_param& fep, uint64_t parent, const string& name, mode_t mode);

#endif //FS_METADATA_OPS_H
