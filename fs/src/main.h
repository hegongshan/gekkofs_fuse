//
// Created by lefthy on 1/24/17.
//

#ifndef MAIN_H
#define MAIN_H

#define FUSE_USE_VERSION 30


#include <fuse3/fuse.h>
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
    std::string rootdir;
    // Below paths are needed for future version when we'll get rid of path hashing.
    std::string inode_path; // used
    std::string dentry_path; // used
    std::string chunk_path; // unused
    std::string mgmt_path;

    // Caching
    std::unordered_map<std::string, std::string> hashmap;
    std::hash<std::string> hashf;

    // Housekeeping
    std::shared_ptr<spdlog::logger> logger;
    uint64_t inode_count;
    std::mutex inode_mutex;
    // Later the blocksize will likely be coupled to the chunks to allow individually big chunk sizes.
    int32_t blocksize;
};

#define ADAFS_DATA ( static_cast<adafs_data*>(fuse_get_context()->private_data))
#define ADAFS_ROOT_INODE 1

namespace util {
    boost::filesystem::path adafs_fullpath(const std::string& path);

    int init_inode_no();

    ino_t generate_inode_no();

    int read_inode_cnt();

    int write_inode_cnt();
}

#endif //MAIN_H
