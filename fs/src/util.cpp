
#include "main.h"

namespace util {

    std::string adafs_fullpath(const std::string &path) {
        return std::string(ADAFS_DATA->rootdir + "/" + path);
    }

    int reset_inode_no() {
        ADAFS_DATA->inode_count = 0;
        return 0;
    }

    ino_t generate_inode_no() {
        std::lock_guard<std::mutex> inode_lock(ADAFS_DATA->inode_mutex);
        return (ino_t) ++ADAFS_DATA->inode_count;
    }


}