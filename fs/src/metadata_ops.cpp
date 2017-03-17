//
// Created by draze on 3/5/17.
//

#include "metadata_ops.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

// TODO error handling. Each read_metadata_field should check for boolean, i.e., if I/O failed.
bool write_all_metadata(const Metadata& md, const unsigned long& hash) {
    write_metadata_field(md.atime(), hash, "/atime"s);
    write_metadata_field(md.mtime(), hash, "/mtime"s);
    write_metadata_field(md.ctime(), hash, "/ctime"s);
    write_metadata_field(md.uid(), hash, "/uid"s);
    write_metadata_field(md.gid(), hash, "/gid"s);
    write_metadata_field(md.mode(), hash, "/mode"s);
    write_metadata_field(md.inode_no(), hash, "/inode_no"s);
    write_metadata_field(md.link_count(), hash, "/link_count"s);
    write_metadata_field(md.size(), hash, "/size"s);
    write_metadata_field(md.blocks(), hash, "/blocks"s);

    return true;
}

// TODO error handling. Each read_metadata_field should check for nullptr, i.e., if I/O failed.
bool read_all_metadata(Metadata& md, const uint64_t& inode, const bfs::path& i_path) {
    md.atime(*read_metadata_field<time_t>(inode, "/atime"s, i_path));
    md.mtime(*read_metadata_field<time_t>(inode, "/mtime"s, i_path));
    md.ctime(*read_metadata_field<time_t>(inode, "/ctime"s, i_path));
    md.uid(*read_metadata_field<uint32_t>(inode, "/uid"s, i_path));
    md.gid(*read_metadata_field<uint32_t>(inode, "/gid"s, i_path));
    md.mode(*read_metadata_field<uint32_t>(inode, "/mode"s, i_path));
    md.inode_no(*read_metadata_field<uint64_t>(inode, "/inode_no"s, i_path));
    md.link_count(*read_metadata_field<uint32_t>(inode, "/link_count"s, i_path));
    md.size(*read_metadata_field<uint32_t>(inode, "/size"s, i_path));
    md.blocks(*read_metadata_field<uint32_t>(inode, "/blocks"s, i_path));
    return true;
}

// TODO error handling.
template<typename T>
bool write_metadata_field(const T& field, const unsigned long& hash, const string& fname) {
    auto path = bfs::path(ADAFS_DATA->inode_path);
    path.append(to_string(hash));
    bfs::create_directories(path);

    path.append(fname);
    // for some reason auto ofs = bfs::ofstream(file); does not work. That is why I use the old style.
    // std::basic_ofstream does not encounter this problem
    bfs::ofstream ofs{path};
    // write to disk in binary form
    boost::archive::binary_oarchive ba(ofs);
    ba << field;

    return true;
}

template<typename T>
unique_ptr<T> read_metadata_field(const uint64_t& inode, const string& fname, bfs::path path) {
    path.append(to_string(inode));
    if (!bfs::exists(path)) return nullptr;

    path.append(fname);
    bfs::ifstream ifs{path};
    //fast error checking
    //ifs.good()
    boost::archive::binary_iarchive ba(ifs);
    auto field = make_unique<T>();
    ba >> *field;
    return field;
}

// Returns the inode for given path. ENOENT if not found
int read_dentry_inode(const std::string path) {
    return 0;
}

int get_metadata(Metadata& md, const std::string& path) {
    return get_metadata(md, bfs::path(path));
}

int get_metadata(Metadata& md, const bfs::path& path) {

//    // TODO HOW TO DO THE LOOKUP PROPERLY? SO NO INODE TYPE CLASH
//    auto inode = lookup(path);
//    ADAFS_DATA->logger->info("get_metadata() inode: {} for path: {}", inode, path);
//    if (inode == -ENOENT) {
//        // If no inode is found lookup father path
//        // <lookup father here>
//        // If still on return inode is not found, return ENOENT
//        return -ENOENT;
//    }
//    auto i_path = util::adafs_fullpath(ADAFS_DATA->inode_path + "/"s + to_string(inode));
//    read_all_metadata(md, inode, i_path);

    return -ENOENT;
}

//int lookup(const string& path) {
//    return lookup(bfs::path(path));
//}
//
int lookup(const bfs::path& path) {
//    if ("/"s.compare(path)) {
//        return ADAFS_ROOT_INODE;
//    } else {
//        return -ENOENT;
//    }
    return 0;
}



