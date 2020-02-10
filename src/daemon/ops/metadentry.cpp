/*
  Copyright 2018-2019, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2019, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/


#include <daemon/ops/metadentry.hpp>
#include <daemon/backend/metadata/db.hpp>
#include <daemon/backend/data/chunk_storage.hpp>

using namespace std;

/**
 * Returns the metadata of an object at a specific path. The metadata can be of dummy values if configured
 * @param path
 * @param attr
 * @return
 */
Metadata gkfs::metadentry::get(const std::string& path) {
    return Metadata(get_str(path));
}

/**
 * Get metadentry string only for path
 * @param path
 * @return
 */
std::string gkfs::metadentry::get_str(const std::string& path) {
    return GKFS_DATA->mdb()->get(path);
}

/**
 * Gets the size of a metadentry
 * @param path
 * @param ret_size (return val)
 * @return err
 */
size_t gkfs::metadentry::get_size(const string& path) {
    return get(path).size();
}

/**
 * Returns a vector of directory entries for given directory
 * @param dir
 * @return
 */
std::vector<std::pair<std::string, bool>> gkfs::metadentry::get_dirents(const std::string& dir) {
    return GKFS_DATA->mdb()->get_dirents(dir);
}

/**
 * Creates metadata (if required) and dentry at the same time
 * @param path
 * @param mode
 */
void gkfs::metadentry::create(const std::string& path, Metadata& md) {

    // update metadata object based on what metadata is needed
    if (GKFS_DATA->atime_state() || GKFS_DATA->mtime_state() || GKFS_DATA->ctime_state()) {
        std::time_t time;
        std::time(&time);
        auto time_s = fmt::format_int(time).str();
        if (GKFS_DATA->atime_state())
            md.atime(time);
        if (GKFS_DATA->mtime_state())
            md.mtime(time);
        if (GKFS_DATA->ctime_state())
            md.ctime(time);
    }
    GKFS_DATA->mdb()->put(path, md.serialize());
}

/**
 * Update metadentry by given Metadata object and path
 * @param path
 * @param md
 */
void gkfs::metadentry::update(const string& path, Metadata& md) {
    GKFS_DATA->mdb()->update(path, path, md.serialize());
}

/**
 * Updates a metadentry's size atomically and returns the corresponding size after update
 * @param path
 * @param io_size
 * @return the updated size
 */
void gkfs::metadentry::update_size(const string& path, size_t io_size, off64_t offset, bool append) {
    GKFS_DATA->mdb()->increase_size(path, io_size + offset, append);
}

/**
 * Remove metadentry if exists and try to remove all chunks for path
 * @param path
 * @return
 */
void gkfs::metadentry::remove_node(const string& path) {
    GKFS_DATA->mdb()->remove(path); // remove metadentry
    GKFS_DATA->storage()->destroy_chunk_space(path); // destroys all chunks for the path on this node
}
