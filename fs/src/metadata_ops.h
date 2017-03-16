//
// Created by draze on 3/5/17.
//

#ifndef FS_METADATA_OPS_H
#define FS_METADATA_OPS_H

#include "main.h"
#include "metadata.h"

using namespace std;

bool write_all_metadata(const Metadata& md, const unsigned long& hash, const boost::filesystem::path& i_path);

bool write_all_metadata(const Metadata& md, const unsigned long& hash, const string& i_path);

bool read_all_metadata(Metadata& md, const uint64_t& inode, const boost::filesystem::path& i_path);

template<typename T>
bool write_metadata_field(const T& field, const unsigned long& hash, const string& fname, boost::filesystem::path path);

template<typename T>
unique_ptr<T> read_metadata_field(const uint64_t& inode, const string& fname, boost::filesystem::path path);

int read_dentry_inode(const std::string path);

int get_metadata(Metadata& md, const std::string& path);

int get_metadata(Metadata& md, const boost::filesystem::path& path);

int lookup(const boost::filesystem::path& path);

#endif //FS_METADATA_OPS_H
