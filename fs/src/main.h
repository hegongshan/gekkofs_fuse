//
// Created by lefthy on 1/24/17.
//

#ifndef MAIN_H
#define MAIN_H

#define FUSE_USE_VERSION 26

#include <fuse/fuse.h>
#include <string>
#include <iostream>
#include <cstdint>

//boost libraries
#include <boost/filesystem.hpp>

#include "spdlog/spdlog.h"


struct adafs_data {
    std::string                             rootdir;
    std::shared_ptr<spdlog::logger>         logger;
    std::int64_t                            inode_count;
    std::mutex                              inode_mutex;
    int32_t                                 blocksize;
};

#define ADAFS_DATA ((struct adafs_data*) fuse_get_context()->private_data)

namespace util {
    std::unique_ptr<std::string> adafs_fullpath(const std::string& path);
    int reset_inode_no(void);
    ino_t generate_inode_no(void);
}

#endif //MAIN_H
