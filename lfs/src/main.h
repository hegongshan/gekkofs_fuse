//
// Created by lefthy on 1/24/17.
//

#ifndef MAIN_H
#define MAIN_H

#define FUSE_USE_VERSION 30


#include <fuse3/fuse_lowlevel.h>
#include <string>
#include <iostream>
#include <cstdint>
#include <unordered_map>

//boost libraries
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "configure.h"
#include "spdlog/spdlog.h"

namespace bfs = boost::filesystem;

struct adafs_data {
    // Caching
    std::unordered_map<std::string, std::string> hashmap;
    std::hash<std::string> hashf;
    // Housekeeping
    uint64_t inode_count;
    std::mutex inode_mutex;
    // Later the blocksize will likely be coupled to the chunks to allow individually big chunk sizes.
    int32_t blocksize;
};

namespace Fs_paths {
    extern std::string rootdir;
    extern std::string inode_path;
    extern std::string dentry_path;
    extern std::string chunk_path;
    extern std::string mgmt_path;
}
// logging instance
extern std::shared_ptr<spdlog::logger> spdlogger;

#define ADAFS_ROOT_INODE 1
// This is the official way to get the userdata from fuse
#define ADAFS_DATA(req) ((struct adafs_data *) fuse_req_userdata(req))


namespace Util {
//    boost::filesystem::path adafs_fullpath(const std::string& path);

    int init_inode_no(struct adafs_data& adata);

    ino_t generate_inode_no(std::mutex& inode_mutex, uint64_t inode_count);

    int read_inode_cnt(struct adafs_data& adafs_data);

    int write_inode_cnt(struct adafs_data& adafs_data);
}

#endif //MAIN_H
