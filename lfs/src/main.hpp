//
// Created by lefthy on 1/24/17.
//

#ifndef MAIN_H
#define MAIN_H

#define FUSE_USE_VERSION 30

extern "C" {
    #include <fuse3/fuse_lowlevel.h>
}
// std libs
#include <string>
#include <iostream>
#include <cstdint>
#include <unordered_map>

// boost libs
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
// adafs
#include "configure.hpp"
// third party libs
#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"
// rocksdb
#include <rocksdb/db.h>
#include <rocksdb/slice.h>
#include <rocksdb/options.h>
#include <rocksdb/utilities/transaction.h>
#include <rocksdb/utilities/optimistic_transaction_db.h>
// margo
extern "C" {
#include <abt.h>
#include <abt-snoozer.h>
#include <abt-io.h>
#include <margo.h>
}


// classes
#include "classes/fs_data.h"

namespace bfs = boost::filesystem;


// XXX This might get moved to the FsData class when I figure out how to use the inode mutex from there...
struct priv_data {

    fuse_ino_t inode_count;
    std::mutex inode_mutex;

};

#define ADAFS_ROOT_INODE static_cast<fuse_ino_t>(1)
#define INVALID_INODE static_cast<fuse_ino_t>(0)
// This is the official way to get the userdata from fuse but its unusable because req has to be dragged everywhere
#define PRIV_DATA(req) (static_cast<priv_data*>(fuse_req_userdata(req)))
#define ADAFS_DATA (static_cast<FsData*>(FsData::getInstance()))

namespace Util {
    int init_inode_no(priv_data& pdata);

    fuse_ino_t generate_inode_no(fuse_req_t& req);

    int read_inode_cnt(priv_data& pdata);

    int write_inode_cnt(priv_data& pdata);
}

#endif //MAIN_H
