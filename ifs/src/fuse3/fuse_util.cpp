
#include "fuse3/fuse_util.hpp"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

using namespace std;

namespace Util {

    int init_inode_no() {
        auto n_hosts = ADAFS_DATA->hosts().size();
        // We are working locally. Start with inode 1
        if (n_hosts == 0) {
            ADAFS_DATA->inode_count(1);
            return 0;
        }
        // hostname was found in given hostlist TODO doublecheck calculation
        auto inode_max_chunk = std::numeric_limits<ino_t>::max();
        auto first_inode = static_cast<ino_t>(((inode_max_chunk / n_hosts) * ADAFS_DATA->host_id()) + 1);
        ADAFS_DATA->inode_count(first_inode);
        return 0;
    }

    fuse_ino_t generate_inode_no() {
        std::lock_guard<std::mutex> inode_lock(ADAFS_DATA->inode_mutex);
        // TODO check that our inode counter is within boundaries of inode numbers in the given node
        return ADAFS_DATA->raise_inode_count(1);
    }

    // XXX error handling
    int read_inode_cnt() {
        auto i_path = bfs::path(ADAFS_DATA->mgmt_path() + "/inode_count");
        bfs::ifstream ifs{i_path};
        boost::archive::binary_iarchive ba(ifs);
        ino_t inode_count;
        ba >> inode_count;
        ADAFS_DATA->inode_count(inode_count);

        return 0;
    }

    // XXX error handling
    int write_inode_cnt() {
        auto i_path = bfs::path(ADAFS_DATA->mgmt_path() + "/inode_count");
        bfs::ofstream ofs{i_path};
        boost::archive::binary_oarchive ba(ofs);
        auto inode = ADAFS_DATA->inode_count();
        ba << inode;

        return 0;
    }

    /**
     * Returns the machine's hostname
     * @return
     */
    string get_my_hostname() {
        char hostname[1024];
        auto ret = gethostname(hostname, 1024);
        return ret == 0 ? string(hostname) : ""s;
    }

}