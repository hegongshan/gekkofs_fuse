//
// Created by evie on 9/8/17.
//

#include <adafs_ops/data.hpp>

using namespace std;

std::string path_to_fspath(const std::string& path) {
    // root path is absolute as is the path comes in here which is hierarchically under root_path
    // XXX check if this can be done easier
    string fs_path;
    set_difference(path.begin(), path.end(), ADAFS_DATA->mountdir().begin(), ADAFS_DATA->mountdir().end(),
                   std::back_inserter(fs_path));
    if (fs_path.at(1) == '/') {
        fs_path = fs_path.substr(1, fs_path.size());
    }
    // replace / with : to avoid making a bunch of mkdirs to store the data in the underlying fs. XXX Can this be done with hashing?
    replace(fs_path.begin(), fs_path.end(), '/', ':');
    return fs_path;
}

/**
 * Creates the directory in the chunk dir for a file to hold data
 * @param inode
 * @return
 */
// XXX this might be just a temp function as long as we don't use chunks
// XXX this function creates not only the chunk folder but also a single file which holds the data of the 'real' file
int init_chunk_space(const std::string& path) {
    auto fs_path = path_to_fspath(path);

    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= fs_path;

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
int destroy_chunk_space(const std::string& path) {
    auto fs_path = path_to_fspath(path);

    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= fs_path;

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
int read_file(char* buf, size_t& read_size, const string& path, const size_t size, const off_t off) {
    auto fs_path = path_to_fspath(path);
    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= fs_path;
    chnk_path /= "data"s;

    int fd = open(chnk_path.c_str(), R_OK);
    if (fd < 0)
        return EIO;
    read_size = static_cast<size_t>(pread(fd, buf, size, off));
    close(fd);
    return 0;
}

int write_file(const string& path, const char* buf, size_t& write_size, const size_t size, const off_t off,
               const bool append) {
    auto fs_path = path_to_fspath(path);
    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= fs_path;
    chnk_path /= "data"s;
    // write to local file
    int fd = open(chnk_path.c_str(), W_OK);
    if (fd < 0)
        return EIO;
    write_size = static_cast<size_t>(pwrite(fd, buf, size, off));
    close(fd);
    // XXX DO WE NEED THE BELOW CODE?
    // Depending on if the file was appended or not metadata sizes need to be modified accordingly
    /*
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
    */
    return 0;
}