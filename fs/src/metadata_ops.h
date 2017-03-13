//
// Created by draze on 3/5/17.
//

#ifndef FS_METADATA_OPS_H
#define FS_METADATA_OPS_H

#include "main.h"
#include "metadata.h"

using namespace std;

bool WriteAllMetadata(const Metadata &md, const std::string &path_str);

bool ReadAllMetadata(Metadata &md, const uint64_t &inode, const std::string &path_str);

template<typename T>
bool WriteMetadataField(const T &field, const uint64_t &inode, const string &fname, const string &p);

template<typename T>
T ReadMetadataField(const uint64_t &inode, const string &fname, const string &p);

#endif //FS_METADATA_OPS_H
