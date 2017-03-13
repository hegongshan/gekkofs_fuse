//
// Created by draze on 3/5/17.
//

#ifndef FS_METADATA_OPS_H
#define FS_METADATA_OPS_H

#include "main.h"
#include "metadata.h"

using namespace std;

bool write_all_metadata(const Metadata &md, const std::string &path_str);

bool read_all_metadata(Metadata &md, const uint64_t &inode, const std::string &path_str);

template<typename T>
bool write_metadata_field(const T &field, const uint64_t &inode, const string &fname, const string &p);

template<typename T>
unique_ptr<T> read_metadata_field(const uint64_t &inode, const string &fname, const string &p);

#endif //FS_METADATA_OPS_H
