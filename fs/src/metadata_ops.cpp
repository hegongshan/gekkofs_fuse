//
// Created by draze on 3/5/17.
//

#include "metadata_ops.h"

namespace fs = boost::filesystem;

// TODO error handling.
bool write_all_metadata(const Metadata& md, const std::string& path_str) {
    write_metadata_field(md.getAtime_(), md.getInode_no(), "/atime"s, path_str);
    write_metadata_field(md.getMtime_(), md.getInode_no(), "/mtime"s, path_str);
    write_metadata_field(md.getCtime_(), md.getInode_no(), "/ctime"s, path_str);
    write_metadata_field(md.getUid(), md.getInode_no(), "/uid"s, path_str);
    write_metadata_field(md.getGid(), md.getInode_no(), "/gid"s, path_str);
    write_metadata_field(md.getMode(), md.getInode_no(), "/mode"s, path_str);
    write_metadata_field(md.getInode_no(), md.getInode_no(), "/inode_no"s, path_str);
    write_metadata_field(md.getLink_count(), md.getInode_no(), "/link_count"s, path_str);
    write_metadata_field(md.getSize(), md.getInode_no(), "/size"s, path_str);
    write_metadata_field(md.getBlocks(), md.getInode_no(), "/blocks"s, path_str);

    return true;
}

bool read_all_metadata(Metadata& md, const uint64_t& inode, const std::string& path_str) {
    md.setAtime_(read_metadata_field(inode, "/atime"s, path_str));
    md.setMtime_(read_metadata_field(inode, "/mtime"s, path_str));
    md.setCtime_(read_metadata_field(inode, "/ctime"s, path_str));
    return true;
}

// TODO error handling.
template<typename T>
bool write_metadata_field(const T& field, const uint64_t& inode, const string& fname, const string& p) {
    auto i_path = p + "/" + to_string(inode);
    fs::create_directories(i_path);
    auto file = fs::path{i_path + fname};
    // for some reason auto ofs = fs::ofstream(file); does not work. That is why I use the old style.
    // std::basic_ofstream does not encounter this problem
    fs::ofstream ofs{file};

    ofs << field;
    return true;
}

//template<typename T>
time_t read_metadata_field(const uint64_t& inode, const string& fname, const string& p) {
    auto i_path = p + "/" + to_string(inode);
    if (!fs::exists(i_path)) return NULL;
    auto file = fs::path(i_path + fname);
    fs::ifstream ifs{file};
    //fast error checking
    //ifs.good()
    auto buf = ""s;
    getline(ifs, buf);
    time_t t = (time_t) stol(buf, nullptr);
    return t;
}

//std::unique_ptr<Metadata> read_metadata(const std::string& path) {
//    return nullptr;
//}
