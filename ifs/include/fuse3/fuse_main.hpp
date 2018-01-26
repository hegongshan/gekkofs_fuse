
#ifndef IFS_FUSE_MAIN_HPP
#define IFS_FUSE_MAIN_HPP

#define FUSE_USE_VERSION 30
extern "C" {
#include <fuse3/fuse_lowlevel.h>
}

#include <memory>
#include <future>
#include <iostream>
//#include <sys/statfs.h>
//#include <stdio.h>
//#include <stdint.h>
//#include <fcntl.h>

// adafs config
#include "configure.hpp"
// boost libs
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
// third party libs
#include <extern/spdlog/spdlog.h>
#include <extern/spdlog/fmt/fmt.h>

extern "C" {
#include <abt.h>
#include <abt-snoozer.h>
#include <mercury_types.h>
#include <mercury_proc_string.h>
#include <margo.h>
}

namespace bfs = boost::filesystem;

#define ADAFS_ROOT_INODE static_cast<fuse_ino_t>(1)
#define INVALID_INODE static_cast<fuse_ino_t>(0)

int fuse_main(int fuse_argc, char* fuse_argv_c[]);

#endif //IFS_FUSE_MAIN_HPP
