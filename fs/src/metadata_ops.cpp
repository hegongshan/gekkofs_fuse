//
// Created by draze on 3/5/17.
//

#include "metadata_ops.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace fs = boost::filesystem;

// TODO error handling. Each read_metadata_field should check for boolean, i.e., if I/O failed.
bool write_all_metadata(const Metadata &md, const std::string &path_str) {
    write_metadata_field(md.atime(), md.inode_no(), "/atime"s, path_str);
    write_metadata_field(md.mtime(), md.inode_no(), "/mtime"s, path_str);
    write_metadata_field(md.ctime(), md.inode_no(), "/ctime"s, path_str);
    write_metadata_field(md.uid(), md.inode_no(), "/uid"s, path_str);
    write_metadata_field(md.gid(), md.inode_no(), "/gid"s, path_str);
    write_metadata_field(md.mode(), md.inode_no(), "/mode"s, path_str);
    write_metadata_field(md.inode_no(), md.inode_no(), "/inode_no"s, path_str);
    write_metadata_field(md.link_count(), md.inode_no(), "/link_count"s, path_str);
    write_metadata_field(md.size(), md.inode_no(), "/size"s, path_str);
    write_metadata_field(md.blocks(), md.inode_no(), "/blocks"s, path_str);

    return true;
}

// TODO error handling. Each read_metadata_field should check for nullptr, i.e., if I/O failed.
bool read_all_metadata(Metadata &md, const uint64_t &inode, const std::string &path_str) {
    md.atime(*read_metadata_field<time_t>(inode, "/atime"s, path_str));
    md.mtime(*read_metadata_field<time_t>(inode, "/mtime"s, path_str));
    md.ctime(*read_metadata_field<time_t>(inode, "/ctime"s, path_str));
    md.uid(*read_metadata_field<uint32_t>(inode, "/uid"s, path_str));
    md.gid(*read_metadata_field<uint32_t>(inode, "/gid"s, path_str));
    md.mode(*read_metadata_field<uint32_t>(inode, "/mode"s, path_str));
    md.inode_no(*read_metadata_field<uint64_t>(inode, "/inode_no"s, path_str));
    md.link_count(*read_metadata_field<uint32_t>(inode, "/link_count"s, path_str));
    md.size(*read_metadata_field<uint32_t>(inode, "/size"s, path_str));
    md.blocks(*read_metadata_field<uint32_t>(inode, "/blocks"s, path_str));
    return true;
}

// TODO error handling.
template<typename T>
bool write_metadata_field(const T &field, const uint64_t &inode, const string &fname, const string &p) {
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
unique_ptr<T> read_metadata_field(const uint64_t &inode, const string &fname, const string &p) {
    auto i_path = p + "/" + to_string(inode);
    if (!fs::exists(i_path)) return nullptr;
    auto file = fs::path(i_path + fname);
    fs::ifstream ifs{file};
    //fast error checking
    //ifs.good()
    boost::archive::binary_iarchive ba(ifs);
    auto field = make_unique<T>();
    ba >> *field;
    return field;
}


