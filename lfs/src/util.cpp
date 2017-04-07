
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "main.h"

namespace Util {

//    bfs::path adafs_fullpath(const std::string& path) {
//        return bfs::path(std::string(ADAFS_DATA->rootdir + "/" + path));
//    }

    int init_inode_no(struct adafs_data& adata) {
        adata.inode_count = 1;
        return 0;
    }

    ino_t generate_inode_no(std::mutex& inode_mutex, uint64_t inode_count) {
        std::lock_guard<std::mutex> inode_lock(inode_mutex);
        return (ino_t) ++inode_count;
    }

    // XXX error handling
    int read_inode_cnt(struct adafs_data& adafs_data) {
        auto i_path = bfs::path(Fs_paths::mgmt_path + "/inode_count");
        bfs::ifstream ifs{i_path};
        boost::archive::binary_iarchive ba(ifs);
        uint64_t inode_count;
        ba >> inode_count;
        adafs_data.inode_count = inode_count;

        return 0;
    }

    // XXX error handling
    int write_inode_cnt(struct adafs_data& adafs_data) {
        auto i_path = bfs::path(Fs_paths::mgmt_path + "/inode_count");
        spdlogger->debug(i_path.string());
        bfs::ofstream ofs{i_path};
        boost::archive::binary_oarchive ba(ofs);
        ba << adafs_data.inode_count;

        return 0;
    }


}