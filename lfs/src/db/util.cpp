//
// Created by evie on 6/8/17.
//

#include "util.hpp"

using namespace std;

/**
 * Build dentry key of form <d_ParentInode_filename>
 * @param inode
 * @param name
 * @return
 */
string db_build_dentry_key(const fuse_ino_t inode, const string& name) {
    return ("d_"s + fmt::FormatInt(inode).str() + "_" + name);
}

/**
 * Build dentry prefix of form <d_ParentInode>
 * @param inode
 * @param name
 * @return
 */
string db_build_dentry_prefix(const fuse_ino_t inode) {
    return ("d_"s + fmt::FormatInt(inode).str() + "_"s);
}

string db_build_dentry_value(const fuse_ino_t inode, const mode_t mode) {
    return (fmt::FormatInt(inode).str() + "_"s + fmt::FormatInt(mode).str());
}

/**
 * Build mdata key of form <inode_fieldname>
 * @param inode
 * @param name
 * @return
 */
string db_build_mdata_key(const fuse_ino_t inode, const string& field) {
    return (fmt::FormatInt(inode).str() + field);
}

string db_build_mdata_key(const string& inode, const string& field) {
    return (inode + field);
}

vector<string> db_build_all_mdata_keys(const fuse_ino_t inode) {
    auto inode_key = fmt::FormatInt(inode).str();
    vector<string> mdata_keys{};
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::atime)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::mtime)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::ctime)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::uid)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::gid)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::mode)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::inode_no)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::link_count)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::size)>(md_field_map));
    mdata_keys.push_back(inode_key + std::get<to_underlying(Md_fields::blocks)>(md_field_map));
    return mdata_keys;
}


