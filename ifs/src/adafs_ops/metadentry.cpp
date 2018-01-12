
#include <adafs_ops/metadentry.hpp>
#include <adafs_ops/data.hpp>
#include <db/db_ops.hpp>

using namespace std;

static const std::string dentry_val_delim = ","s; // XXX this needs to be global.

ino_t generate_inode_no() {
    std::lock_guard<std::mutex> inode_lock(ADAFS_DATA->inode_mutex);
    // TODO check that our inode counter is within boundaries of inode numbers in the given node
    return ADAFS_DATA->raise_inode_count(1);
}

/**
 * Creates a file system node of any type (such as file or directory)
 * @param path
 * @param uid
 * @param gid
 * @param mode
 * @return
 */
int create_node(const std::string& path, const uid_t uid, const gid_t gid, mode_t mode) {
    auto err = create_metadentry(path, mode); // XXX errorhandling

    // XXX Only do that for files and not for directories
    init_chunk_space(path);

    return err;
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

    return db_put_metadentry(path, md.to_KVentry()) ? 0 : -1;
}

/**
 * Returns the metadata of an object at a specific path. The metadata can be of dummy values if configured
 * @param path
 * @param attr
 * @return
 */
int get_metadentry(const std::string& path, Metadata& md) {
    string val;
    auto err = db_get_metadentry(path, val);
    if (!err || val.size() == 0) {
        return -1;
    }
    Metadata mdi{path, val};
    md = mdi;
    return 0;
}

int remove_metadentry(const string& path) {
    return db_delete_metadentry(path) ? 0 : -1;
}

int remove_node(const string& path) {
    auto err = remove_metadentry(path);
    // XXX Only do that with a file. Directory needs to be handled differently
    if (err == 0)
        destroy_chunk_space(
                path); // XXX This removes only the data on that node. Leaving everything in inconsistent state
    return err;
}

/**
 * Updates a metadentry's size atomically and returns the corresponding size after update
 * @param path
 * @param io_size
 * @return the updated size
 */
int update_metadentry_size(const string& path, size_t io_size, off_t offset, bool append, size_t& read_size) {
    // XXX This function has to be completely atomic. Do we need transactions here? or a separate locking db?
#ifdef LOG_TRACE
    db_iterate_all_entries();
#endif
    string val;
    auto err = db_get_metadentry(path, val);
    if (!err || val.size() == 0) {
        return ENOENT;
    }
    Metadata md{path, val};
    if (static_cast<unsigned long>(offset) > md.size()) // Writing beyond file dimensions is prohibited for now
        return EFAULT;
    // update io_size
    if (append)
        md.size(md.size() + io_size);
    else { // if no append but io_size exceeds the file's size, update the size correspondingly
        if (io_size + static_cast<unsigned long>(offset) > md.size())
            md.size(io_size + offset);
        else {
            read_size = md.size();
            return 0;
        }
    }
    read_size = db_update_metadentry(path, path, md.to_KVentry()) ? md.size() : -1; // update database atomically
#ifdef LOG_TRACE
    db_iterate_all_entries();
#endif
    return 0;
}

int update_metadentry(const string& path, Metadata& md) {
    return db_update_metadentry(path, md.path(), md.to_KVentry()) ? 0 : -1;
}