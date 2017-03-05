
#include "main.h"

namespace util {


    int reset_inode_no(void) {
        ADAFS_DATA->inode_count = 0;
        return 0;
    }

    ino_t generate_inode_no(void) {
        std::lock_guard<std::mutex> inode_lock(ADAFS_DATA->inode_mutex);
        return (ino_t) ADAFS_DATA->inode_count++;
    }
}