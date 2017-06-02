
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "main.hpp"

namespace Util {

    int init_inode_no(priv_data& pdata) {
        pdata.inode_count = 1;
        return 0;
    }

    fuse_ino_t generate_inode_no(fuse_req_t& req) {
        std::lock_guard<std::mutex> inode_lock(PRIV_DATA(req)->inode_mutex);
        return static_cast<fuse_ino_t>(++PRIV_DATA(req)->inode_count);
    }

    // XXX error handling
    int read_inode_cnt(priv_data& pdata) {
        auto i_path = bfs::path(ADAFS_DATA->mgmt_path() + "/inode_count");
        bfs::ifstream ifs{i_path};
        boost::archive::binary_iarchive ba(ifs);
        fuse_ino_t inode_count;
        ba >> inode_count;
        pdata.inode_count = inode_count;

        return 0;
    }

    // XXX error handling
    int write_inode_cnt(priv_data& pdata) {
        auto i_path = bfs::path(ADAFS_DATA->mgmt_path() + "/inode_count");
        bfs::ofstream ofs{i_path};
        boost::archive::binary_oarchive ba(ofs);
        ba << pdata.inode_count;

        return 0;
    }


}