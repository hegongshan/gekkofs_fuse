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
#include "classes/fs_data.h"

namespace bfs = boost::filesystem;


// XXX This might get moved to the FsData class when I figure out how to use the inode mutex from there...
struct priv_data {

    uint64_t inode_count;
    std::mutex inode_mutex;

};

#define ADAFS_ROOT_INODE 1
// This is the official way to get the userdata from fuse but its unusable because req has to be dragged everywhere
#define PRIV_DATA(req) ((struct priv_data *) fuse_req_userdata(req))
#define ADAFS_DATA ((FsData*) FsData::getInstance())

namespace Util {
    int init_inode_no(priv_data& pdata);

    ino_t generate_inode_no(std::mutex& inode_mutex, uint64_t inode_count);

    int read_inode_cnt(priv_data& pdata);

    int write_inode_cnt(priv_data& pdata);
}

#endif //MAIN_H
