
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "main.h"

namespace util {

    bfs::path adafs_fullpath(const std::string& path) {
        return bfs::path(std::string(ADAFS_DATA->rootdir + "/" + path));
    }

    int init_inode_no() {
        ADAFS_DATA->inode_count = 1;
        return 0;
    }

    ino_t generate_inode_no() {
        std::lock_guard<std::mutex> inode_lock(ADAFS_DATA->inode_mutex);
        return (ino_t) ++ADAFS_DATA->inode_count;
    }

    // XXX error handling
    int read_inode_cnt() {
        auto i_path = bfs::path(ADAFS_DATA->mgmt_path + "/inode_count");
        bfs::ifstream ifs{i_path};
        boost::archive::binary_iarchive ba(ifs);
        uint64_t inode_count;
        ba >> inode_count;
        ADAFS_DATA->inode_count = inode_count;

        return 0;
    }

    // XXX error handling
    int write_inode_cnt() {
        auto i_path = bfs::path(ADAFS_DATA->mgmt_path + "/inode_count");
        bfs::ofstream ofs{i_path};
        boost::archive::binary_oarchive ba(ofs);
        ba << ADAFS_DATA->inode_count;

        return 0;
    }


}