//
// Created by evie on 4/3/17.
//

#include "io.hpp"

using namespace std;

/**
 * Creates the directory in the chunk dir for a file to hold data
 * @param inode
 * @return
 */
// XXX this might be just a temp function as long as we don't use chunks
// XXX this function creates not only the chunk folder but also a single file which holds the data of the 'real' file
int init_chunk_space(const fuse_ino_t inode) {
    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= to_string(inode);

    // create chunk dir
    bfs::create_directories(chnk_path);

    // XXX create temp big file. remember also to modify the return value
    chnk_path /= "data"s;
    bfs::ofstream ofs{chnk_path};

//    return static_cast<int>(bfs::exists(chnk_path));
    return 0;
}
/**
 * Remove the directory in the chunk dir of a file.
 * @param inode
 * @return
 */
// XXX this might be just a temp function as long as we don't use chunks
int destroy_chunk_space(const fuse_ino_t inode) {
    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= to_string(inode);

    // create chunk dir
    bfs::remove_all(chnk_path);

//    return static_cast<int>(!bfs::exists(chnk_path));
    return 0;
}

int read_file(char* buf, size_t& read_size, const char* path, const size_t size, const off_t off) {
    int fd = open(path, R_OK);
    if (fd < 0)
        return EIO;
    read_size = static_cast<size_t>(pread(fd, buf, size, off));
    close(fd);
    return 0;
}