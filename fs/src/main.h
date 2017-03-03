//
// Created by lefthy on 1/24/17.
//

#ifndef MAIN_H
#define MAIN_H

#define FUSE_USE_VERSION 26

#include <fuse/fuse.h>
#include <string>
#include "spdlog/spdlog.h"

struct adafs_data {
    std::string         rootdir;
    std::shared_ptr<spdlog::logger>       logger;
};

#define ADAFS_DATA ((struct adafs_data*) fuse_get_context()->private_data)

#endif //MAIN_H
