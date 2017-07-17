//
// Created by evie on 4/3/17.
//

#include "io.hpp"
#include "../classes/metadata.hpp"
#include "mdata_ops.hpp"

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

/**
 * pread wrapper
 * @param buf
 * @param read_size
 * @param path
 * @param size
 * @param off
 * @return
 */
int read_file(char* buf, size_t& read_size, const char* path, const size_t size, const off_t off) {
    int fd = open(path, R_OK);
    if (fd < 0)
        return EIO;
    read_size = static_cast<size_t>(pread(fd, buf, size, off));
    close(fd);
    return 0;
}

int write_file(const fuse_ino_t inode, const char *buf, size_t &write_size, const size_t size, const off_t off,
               const bool append) {
    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= fmt::FormatInt(inode).c_str();
    chnk_path /= "data"s;
    // write to local file
    int fd = open(chnk_path.c_str(), W_OK);
    if (fd < 0)
        return EIO;
    write_size = static_cast<size_t>(pwrite(fd, buf, size, off));
    close(fd);
    // Depending on if the file was appended or not metadata sizes need to be modified accordingly
    if (append) {
        // appending requires to read the old size first so that the new size can be added to it
        Metadata md{};
        read_metadata_field_md(inode, Md_fields::size, md);
        // truncating file
        truncate(chnk_path.c_str(), md.size() + size);
        // refresh metadata size field
        write_metadata_field(inode, Md_fields::size, md.size() + static_cast<off_t>(size));
    } else {
        truncate(chnk_path.c_str(), size);
        write_metadata_field(inode, Md_fields::size, static_cast<off_t>(size));
    }

    return 0;
}
