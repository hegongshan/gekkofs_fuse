#include <daemon/ops/metadentry.hpp>
#include <daemon/backend/metadata/db.hpp>
#include <daemon/backend/data/chunk_storage.hpp>

using namespace std;

namespace gkfs::metadata {

/**
 * Returns the metadata of an object at a specific path. The metadata can be of
 * dummy values if configured
 * @param path
 * @param attr
 * @return
 */
Metadata
get(const std::string& path) {
    return Metadata(get_str(path));
}

/**
 * Get metadentry string only for path
 * @param path
 * @return
 */
std::string
get_str(const std::string& path) {
    return GKFS_DATA->mdb()->get(path);
}

/**
 * Gets the size of a metadentry
 * @param path
 * @param ret_size (return val)
 * @return err
 */
size_t
get_size(const string& path) {
    return get(path).size();
}

/**
 * Returns a vector of directory entries for given directory
 * @param dir
 * @return
 */
std::vector<std::pair<std::string, bool>>
get_dirents(const std::string& dir) {
    return GKFS_DATA->mdb()->get_dirents(dir);
}

/**
 * Returns a vector of directory entries for given directory (extended version)
 * @param dir
 * @return
 */
std::vector<std::tuple<std::string, bool, size_t, time_t>>
get_dirents_extended(const std::string& dir) {
    return GKFS_DATA->mdb()->get_dirents_extended(dir);
}


/**
 * Creates metadata (if required) and dentry at the same time
 * @param path
 * @param mode
 * @throws DBException
 */
void
create(const std::string& path, Metadata& md) {

    // update metadata object based on what metadata is needed
    if(GKFS_DATA->atime_state() || GKFS_DATA->mtime_state() ||
       GKFS_DATA->ctime_state()) {
        std::time_t time;
        std::time(&time);
        auto time_s = fmt::format_int(time).str();
        if(GKFS_DATA->atime_state())
            md.atime(time);
        if(GKFS_DATA->mtime_state())
            md.mtime(time);
        if(GKFS_DATA->ctime_state())
            md.ctime(time);
    }
    if(gkfs::config::metadata::create_exist_check) {
        GKFS_DATA->mdb()->put_no_exist(path, md.serialize());
    } else {
        GKFS_DATA->mdb()->put(path, md.serialize());
    }
}

/**
 * Update metadentry by given Metadata object and path
 * @param path
 * @param md
 */
void
update(const string& path, Metadata& md) {
    GKFS_DATA->mdb()->update(path, path, md.serialize());
}

/**
 * Updates a metadentry's size atomically and returns the corresponding size
 * after update
 * @param path
 * @param io_size
 * @return the updated size
 */
void
update_size(const string& path, size_t io_size, off64_t offset, bool append) {
    GKFS_DATA->mdb()->increase_size(path, io_size + offset, append);
}

/**
 * Remove metadentry if exists
 * @param path
 * @return
 * @throws gkfs::metadata::DBException
 */
void
remove(const string& path) {
    /*
     * try to remove metadata from kv store but catch NotFoundException which is
     * not an error in this case because removes can be broadcast to catch all
     * data chunks but only one node will hold the kv store entry.
     */
    try {
        GKFS_DATA->mdb()->remove(path); // remove metadata from KV store
    } catch(const NotFoundException& e) {
    }
}

} // namespace gkfs::metadata
