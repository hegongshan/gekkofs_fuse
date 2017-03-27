//
// Created by draze on 3/5/17.
//

#include "metadata_ops.h"
#include "dentry_ops.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

// TODO error handling. Each read_metadata_field should check for boolean, i.e., if I/O failed.
bool write_all_metadata(const Metadata& md, const unsigned long hash) {
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

// TODO error handling.
template<typename T>
bool write_metadata_field(const T& field, const unsigned long hash, const string& leaf_name) {
    auto i_path = bfs::path(ADAFS_DATA->inode_path);
    i_path /= to_string(hash);
    bfs::create_directories(i_path);

    i_path /= leaf_name;
    // for some reason auto ofs = bfs::ofstream(file); does not work. That is why I use the old style.
    // std::basic_ofstream does not encounter this problem
    bfs::ofstream ofs{i_path};
    // write to disk in binary form
    boost::archive::binary_oarchive ba(ofs);
    ba << field;

    return true;
}

// TODO error handling. Each read_metadata_field should check for nullptr, i.e., if I/O failed.
bool read_all_metadata(Metadata& md, const unsigned long hash) {
    md.atime(*read_metadata_field<time_t>(hash, "/atime"s));
    md.mtime(*read_metadata_field<time_t>(hash, "/mtime"s));
    md.ctime(*read_metadata_field<time_t>(hash, "/ctime"s));
    md.uid(*read_metadata_field<uint32_t>(hash, "/uid"s));
    md.gid(*read_metadata_field<uint32_t>(hash, "/gid"s));
    md.mode(*read_metadata_field<uint32_t>(hash, "/mode"s));
    md.inode_no(*read_metadata_field<uint64_t>(hash, "/inode_no"s));
    md.link_count(*read_metadata_field<uint32_t>(hash, "/link_count"s));
    md.size(*read_metadata_field<uint32_t>(hash, "/size"s));
    md.blocks(*read_metadata_field<uint32_t>(hash, "/blocks"s));
    return true;
}


template<typename T>
unique_ptr<T> read_metadata_field(const uint64_t hash, const string& leaf_name) {
    auto path = bfs::path(ADAFS_DATA->inode_path);
    path /= to_string(hash);
    path /= leaf_name;
    if (!bfs::exists(path)) return nullptr;

    bfs::ifstream ifs{path};
    //fast error checking
    //ifs.good()
    boost::archive::binary_iarchive ba(ifs);
    auto field = make_unique<T>();
    ba >> *field;
    return field;
}

int get_metadata(Metadata& md, const string& path) {
    return get_metadata(md, bfs::path(path));
}

int get_metadata(Metadata& md, const bfs::path& path) {
    ADAFS_DATA->logger->debug("get_metadata() enter for path {}", path.string());
    // Verify that the file is a valid dentry of the parent dir
    if (verify_dentry(path)) {
        // Metadata for file exists
        read_all_metadata(md, ADAFS_DATA->hashf(path.string()));
        return 0;
    } else {
        return -ENOENT;
    }
}

/**
 * Reads all directory entries in a directory with a given @hash. Returns 0 if successful.
 * @dir is assumed to be empty
 */
int read_dentries(vector<string> dir, const unsigned long hash) {
    auto path = bfs::path(ADAFS_DATA->dentry_path);
    path /= to_string(hash);
    if (!bfs::exists(path)) return 1;
    // shortcut if path is empty = no files in directory
    if (bfs::is_empty(path)) return 0;

    // Below can be simplified with a C++11 range based loop? But how? :( XXX
    bfs::directory_iterator end_dir_it;
    for (bfs::directory_iterator dir_it(path); dir_it != end_dir_it; ++dir_it) {
        const bfs::path cp = (*dir_it);
        dir.push_back(cp.string());
    }
    return 0;
}






