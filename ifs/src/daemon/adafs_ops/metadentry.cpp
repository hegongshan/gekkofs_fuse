
#include <daemon/adafs_ops/metadentry.hpp>
#include <daemon/adafs_ops/data.hpp>
#include <daemon/backend/metadata/db.hpp>

using namespace std;

ino_t generate_inode_no() {
    std::lock_guard<std::mutex> inode_lock(ADAFS_DATA->inode_mutex);
    // TODO check that our inode counter is within boundaries of inode numbers in the given node
    return ADAFS_DATA->raise_inode_count(1);
}

/**
 * Creates metadata (if required) and dentry at the same time
 * @param path
 * @param mode
 * @return
 */
int create_metadentry(const std::string& path, mode_t mode) {

    Metadata md{path, mode};
    // update metadata object based on what metadata is needed
    if (ADAFS_DATA->atime_state() || ADAFS_DATA->mtime_state() || ADAFS_DATA->ctime_state()) {
        std::time_t time;
        std::time(&time);
        auto time_s = fmt::FormatInt(time).str();
        if (ADAFS_DATA->atime_state())
            md.atime(time);
        if (ADAFS_DATA->mtime_state())
            md.mtime(time);
        if (ADAFS_DATA->ctime_state())
            md.ctime(time);
    }
    if (ADAFS_DATA->uid_state())
        md.uid(getuid());
    if (ADAFS_DATA->gid_state())
        md.gid(getgid());
    if (ADAFS_DATA->inode_no_state())
        md.inode_no(generate_inode_no());

    return ADAFS_DATA->mdb()->put(path, md.serialize()) ? 0 : -1;
}

std::string get_metadentry_str(const std::string& path) {
        return ADAFS_DATA->mdb()->get(path);
}

/**
 * Returns the metadata of an object at a specific path. The metadata can be of dummy values if configured
 * @param path
 * @param attr
 * @return
 */
Metadata get_metadentry(const std::string& path) {
    return {path, get_metadentry_str(path)};
}

/**
 * Wrapper to remove a KV store entry with the path as key
 * @param path
 * @return
 */
int remove_metadentry(const string& path) {
    return ADAFS_DATA->mdb()->remove(path) ? 0 : -1;
}

/**
 * Remove metadentry if exists and try to remove all chunks for path
 * @param path
 * @return
 */
int remove_node(const string& path) {
    int err = 0; // assume we succeed
    Metadata md{};
    // If metadentry exists, try to remove it
    if (get_metadentry(path, md) == 0) {
        err = remove_metadentry(path);
    }
    destroy_chunk_space(path); // destroys all chunks for the path on this node
    return err;
}

/**
 * Gets the size of a metadentry
 * @param path
 * @param ret_size (return val)
 * @return err
 */
size_t get_metadentry_size(const string& path) {
    return get_metadentry(path).size();
}

/**
 * Updates a metadentry's size atomically and returns the corresponding size after update
 * @param path
 * @param io_size
 * @return the updated size
 */
int update_metadentry_size(const string& path, size_t io_size, off64_t offset, bool append,  size_t& read_size) {
#ifdef LOG_TRACE
    ADAFS_DATA->mdb()->iterate_all();
#endif
    auto err = ADAFS_DATA->mdb()->update_size(path, io_size, offset, append);
    if (!err) {
        return EBUSY;
    }
#ifdef LOG_TRACE
    ADAFS_DATA->mdb()->iterate_all();
#endif
    //XXX This breaks append writes, needs to be fixed
    read_size = 0;
    return 0;
}

int update_metadentry(const string& path, Metadata& md) {
    return ADAFS_DATA->mdb()->update(path, md.path(), md.serialize()) ? 0 : -1;
}

/**
 * @param path of the object whose permissions are checked
 * @param mask single bit mask to check against
 * @return errno
 */
int check_access_mask(const string& path, const int mask) {
    Metadata md;
    try {
        md = get_metadentry(path);
    } catch (const NotFoundException& e) {
        return ENOENT;
    }

    /*
     * if only check if file exists is wanted, return success.
     * According to POSIX (access(2)) the mask is either the value F_OK,
     * or a mask consisting of the bitwise OR of one or more of R_OK, W_OK, and X_OK.
     */
    if (mask == F_OK)
        return 0;

    // root user is a god
    if (ADAFS_DATA->uid_state() && md.uid() == 0)
        return 0;

    // We do not check for the actual user here, because the cluster batchsystem should take care of it
    // check user leftmost 3 bits for rwx in md->mode
    if (ADAFS_DATA->uid_state()) {
        // Because mode comes only with the first 3 bits used, the user bits have to be shifted to the right to compare
        if ((mask & md.mode() >> 6) == static_cast<unsigned int>(mask))
            return 0;
        else
            return EACCES;
    }

    // check group middle 3 bits for rwx in md->mode
    if (ADAFS_DATA->gid_state()) {
        if ((mask & md.mode() >> 3) == static_cast<unsigned int>(mask))
            return 0;
        else
            return EACCES;
    }

    // check other rightmost 3 bits for rwx in md->mode.
    // Because they are the rightmost bits they don't need to be shifted
    if ((mask & md.mode()) == static_cast<unsigned int>(mask)) {
        return 0;
    }

    return EACCES;
}