//
// Created by draze on 3/5/17.
//

#include "metadata_ops.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace fs = boost::filesystem;

// TODO error handling.
bool WriteAllMetadata(const Metadata &md, const std::string &path_str) {
    WriteMetadataField(md.getAtime(), md.getInode_no(), "/atime"s, path_str);
    WriteMetadataField(md.getMtime(), md.getInode_no(), "/mtime"s, path_str);
    WriteMetadataField(md.getCtime(), md.getInode_no(), "/ctime"s, path_str);
    WriteMetadataField(md.getUid(), md.getInode_no(), "/uid"s, path_str);
    WriteMetadataField(md.getGid(), md.getInode_no(), "/gid"s, path_str);
    WriteMetadataField(md.getMode(), md.getInode_no(), "/mode"s, path_str);
    WriteMetadataField(md.getInode_no(), md.getInode_no(), "/inode_no"s, path_str);
    WriteMetadataField(md.getLink_count(), md.getInode_no(), "/link_count"s, path_str);
    WriteMetadataField(md.getSize(), md.getInode_no(), "/size"s, path_str);
    WriteMetadataField(md.getBlocks(), md.getInode_no(), "/blocks"s, path_str);

    return true;
}

bool ReadAllMetadata(Metadata &md, const uint64_t &inode, const std::string &path_str) {
    md.setAtime(ReadMetadataField<time_t>(inode, "/atime"s, path_str));
    md.setMtime(ReadMetadataField<time_t>(inode, "/mtime"s, path_str));
    md.setCtime(ReadMetadataField<time_t>(inode, "/ctime"s, path_str));
    return true;
}

// TODO error handling.
template<typename T>
bool WriteMetadataField(const T &field, const uint64_t &inode, const string &fname, const string &p) {
    auto i_path = p + "/" + to_string(inode);
    fs::create_directories(i_path);
    auto file = fs::path{i_path + fname};
    // for some reason auto ofs = fs::ofstream(file); does not work. That is why I use the old style.
    // std::basic_ofstream does not encounter this problem
    fs::ofstream ofs{file};
    // write to disk in binary form
    boost::archive::binary_oarchive ba(ofs);
    ba << field;

    return true;
}

template<typename T>
T ReadMetadataField(const uint64_t &inode, const string &fname, const string &p) {
    auto i_path = p + "/" + to_string(inode);
//    if (!fs::exists(i_path)) return nullptr;
    auto file = fs::path(i_path + fname);
    fs::ifstream ifs{file};
    //fast error checking
    //ifs.good()
    boost::archive::binary_iarchive ba(ifs);
    T field;
    ba >> field;
    return field;
}


