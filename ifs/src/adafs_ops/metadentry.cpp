//
// Created by evie on 9/6/17.
//

#include <adafs_ops/metadentry.hpp>
#include <adafs_ops/data.hpp>
#include <db/db_ops.hpp>

using namespace std;

static const std::string dentry_val_delim = ","s;

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
    auto val = fmt::FormatInt(
            mode).str(); // For now just the mode is the value. Later any metadata combination can be in there. TODO

    // The order is important. don't change.
    if (ADAFS_DATA->atime_state() || ADAFS_DATA->mtime_state() || ADAFS_DATA->ctime_state()) {
        std::time_t time;
        std::time(&time);
        auto time_s = fmt::FormatInt(time).str();
        if (ADAFS_DATA->atime_state()) {
            val += dentry_val_delim + time_s;
        }
        if (ADAFS_DATA->mtime_state()) {
            val += dentry_val_delim + time_s;
        }
        if (ADAFS_DATA->ctime_state()) {
            val += dentry_val_delim + time_s;
        }
    }
    if (ADAFS_DATA->uid_state()) {
        val += dentry_val_delim + fmt::FormatInt(getuid()).str();
    }
    if (ADAFS_DATA->gid_state()) {
        val += dentry_val_delim + fmt::FormatInt(getgid()).str();
    }
    if (ADAFS_DATA->inode_no_state()) {
        val += dentry_val_delim + fmt::FormatInt(Util::generate_inode_no()).str();
    }
    if (ADAFS_DATA->link_cnt_state()) {
        val += dentry_val_delim + "1"s;
    }
    if (ADAFS_DATA->blocks_state()) {
        val += dentry_val_delim + "0"s;
    }

    return db_put_metadentry(path, val) ? 0 : -1;
}

///**
// * Converts the dentry db value into a stat struct, which is needed by Linux
// * @param path
// * @param db_val
// * @param attr
// * @return
// */
//int db_val_to_stat(const std::string& path, std::string db_val, struct stat& attr) {
//
//    auto pos = db_val.find(dentry_val_delim);
//    if (pos == std::string::npos) { // no delimiter found => no metadata enabled. fill with dummy values
//        attr.st_ino = ADAFS_DATA->hashf()(path);
//        attr.st_mode = static_cast<unsigned int>(stoul(db_val));
//        attr.st_nlink = 1;
//        attr.st_uid = getuid();
//        attr.st_gid = getgid();
//        attr.st_size = 0;
//        attr.st_blksize = ADAFS_DATA->blocksize();
//        attr.st_blocks = 0;
//        attr.st_atim.tv_sec = 0;
//        attr.st_mtim.tv_sec = 0;
//        attr.st_ctim.tv_sec = 0;
//        return 0;
//    }
//    // some metadata is enabled
//    attr.st_mode = static_cast<unsigned int>(stoul(db_val.substr(0, pos)));
//    db_val.erase(0, pos + 1);
//    // The order is important. don't change.
//    if (ADAFS_DATA->atime_state()) {
//        pos = db_val.find(dentry_val_delim);
//        attr.st_atim.tv_sec = static_cast<time_t>(stol(db_val.substr(0, pos)));
//        db_val.erase(0, pos + 1);
//    }
//    if (ADAFS_DATA->mtime_state()) {
//        pos = db_val.find(dentry_val_delim);
//        attr.st_mtim.tv_sec = static_cast<time_t>(stol(db_val.substr(0, pos)));
//        db_val.erase(0, pos + 1);
//    }
//    if (ADAFS_DATA->ctime_state()) {
//        pos = db_val.find(dentry_val_delim);
//        attr.st_ctim.tv_sec = static_cast<time_t>(stol(db_val.substr(0, pos)));
//        db_val.erase(0, pos + 1);
//    }
//    if (ADAFS_DATA->uid_state()) {
//        pos = db_val.find(dentry_val_delim);
//        attr.st_uid = static_cast<uid_t>(stoul(db_val.substr(0, pos)));
//        db_val.erase(0, pos + 1);
//    }
//    if (ADAFS_DATA->gid_state()) {
//        pos = db_val.find(dentry_val_delim);
//        attr.st_gid = static_cast<uid_t>(stoul(db_val.substr(0, pos)));
//        db_val.erase(0, pos + 1);
//    }
//    if (ADAFS_DATA->inode_no_state()) {
//        pos = db_val.find(dentry_val_delim);
//        attr.st_ino = static_cast<ino_t>(stoul(db_val.substr(0, pos)));
//        db_val.erase(0, pos + 1);
//    }
//    if (ADAFS_DATA->link_cnt_state()) {
//        pos = db_val.find(dentry_val_delim);
//        attr.st_nlink = static_cast<nlink_t>(stoul(db_val.substr(0, pos)));
//        db_val.erase(0, pos + 1);
//    }
//    if (ADAFS_DATA->blocks_state()) { // last one will not encounter a delimiter anymore
//        attr.st_blocks = static_cast<blkcnt_t>(stoul(db_val));
//    }
//    return 0;
//}

/**
 * Returns the metadata of an object at a specific path. The metadata can be of dummy values if configured
 * @param path
 * @param attr
 * @return
 */
std::string get_attr(const std::string& path) {
    return db_get_metadentry(path);
//    db_val_to_stat(path, val, *attr);
//    return 0;
}

int remove_metadentry(const string& path) {
    return db_delete_metadentry(path) ? 0 : -1;
}

int remove_node(const string& path) {
    auto err = remove_metadentry(path);
    if (err == 0)
        destroy_chunk_space(path);
    return err;
}