//
// Created by draze on 3/5/17.
//

#ifndef FS_METADATA_OPS_H
#define FS_METADATA_OPS_H

#include "../main.h"
#include "../classes/metadata.h"

using namespace std;

// mapping of enum to string to get the file names for metadata
enum class Md_fields { atime, mtime, ctime, uid, gid, mode, inode_no, link_count, size, blocks };

const std::map<Md_fields, std::string> md_field_map = {
        {Md_fields::atime, "/atime"},
        {Md_fields::mtime, "/mtime"},
        {Md_fields::ctime, "/ctime"},
        {Md_fields::uid, "/uid"},
        {Md_fields::gid, "/gid"},
        {Md_fields::mode, "/mode"},
        {Md_fields::inode_no, "/inode_no"},
        {Md_fields::link_count, "/link_count"},
        {Md_fields::size, "/size"},
        {Md_fields::blocks, "/blocks"}
};

bool write_all_metadata(const Metadata& md, const unsigned long hash);

template<typename T>
bool write_metadata_field(const T& field, const unsigned long hash, const string& leaf_name);

bool read_all_metadata(Metadata& md, const unsigned long hash);

template<typename T>
unique_ptr<T> read_metadata_field(const unsigned long hash, const string& leaf_name);

int get_metadata(Metadata& md, const std::string& path);

int get_metadata(Metadata& md, const boost::filesystem::path& path);

int remove_metadata(const unsigned long hash);

#endif //FS_METADATA_OPS_H
