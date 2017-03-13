
#include "main.h"

namespace util {

    std::string AdafsFullpath(const std::string &path) {
        return std::string(ADAFS_DATA->rootdir + "/" + path);
    }

    int ResetInodeNo(void) {
        ADAFS_DATA->inode_count = 0;
        return 0;
    }

    ino_t GenerateInodeNo(void) {
        std::lock_guard<std::mutex> inode_lock(ADAFS_DATA->inode_mutex);
        return (ino_t) ++ADAFS_DATA->inode_count;
    }


}