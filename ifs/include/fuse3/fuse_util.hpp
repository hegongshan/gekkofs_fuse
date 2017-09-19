//
// Created by evie on 9/19/17.
//

#ifndef IFS_FUSE_UTIL_HPP
#define IFS_FUSE_UTIL_HPP

#include <fuse3/fuse_main.hpp>

namespace Util {
    int init_inode_no();

    fuse_ino_t generate_inode_no();

    int read_inode_cnt();

    int write_inode_cnt();

    std::string get_my_hostname();
}

#endif //IFS_FUSE_UTIL_HPP
