//
// Created by evie on 9/6/17.
//

#include "adafs_ops/metadentry.hpp"
#include "db/db_ops.hpp"

using namespace std;

int create_node(const std::string& path, const uid_t uid, const gid_t gid, mode_t mode) {
    create_metadentry(path, mode); // XXX errorhandling

    return 0;
}

int create_metadentry(const std::string& path, mode_t mode) {
    auto val = fmt::FormatInt(
            mode).str(); // For now just the mode is the value. Later any metadata combination can be in there. TODO

    // The order is important. don't change.
    if (ADAFS_DATA->atime_state() || ADAFS_DATA->mtime_state() || ADAFS_DATA->ctime_state()) {
        std::time_t time;
        std::time(&time);
        auto time_s = fmt::FormatInt(time).str();
        if (ADAFS_DATA->atime_state()) {
            val += ","s + time_s;
        }
        if (ADAFS_DATA->mtime_state()) {
            val += ","s + time_s;
        }
        if (ADAFS_DATA->ctime_state()) {
            val += ","s + time_s;
        }
    }
    if (ADAFS_DATA->uid_state()) {
        val += ","s + fmt::FormatInt(getuid()).str();
    }
    if (ADAFS_DATA->gid_state()) {
        val += ","s + fmt::FormatInt(getgid()).str();
    }
    if (ADAFS_DATA->inode_no_state()) {
        val += ","s + fmt::FormatInt(Util::generate_inode_no()).str();
    }
    if (ADAFS_DATA->link_cnt_state()) {
        val += ",1"s;
    }
    if (ADAFS_DATA->blocks_state()) {
        val += ",0";
    }

    return db_put_metadentry(path, val);
}

int db_metadentry_to_stat(const std::string& path, struct stat& attr) {
    auto val = db_get_metadentry(path);

    auto delim = ","s;
    auto pos = val.find(delim);
    if (pos == std::string::npos) { // no delimiter found => no metadata enabled. fill with dummy values
        attr.st_ino = ADAFS_DATA->hashf()(path);
        attr.st_mode = static_cast<unsigned int>(stoul(db_get_metadentry(path)));
        attr.st_nlink = 1;
        attr.st_uid = getuid();
        attr.st_gid = getgid();
        attr.st_size = 0;
        attr.st_blksize = ADAFS_DATA->blocksize();
        attr.st_blocks = 0;
        attr.st_atim.tv_sec = 0;
        attr.st_mtim.tv_sec = 0;
        attr.st_ctim.tv_sec = 0;
        return 0;
    }
    // some metadata is enabled
    attr.st_mode = static_cast<unsigned int>(stoul(val.substr(0, pos)));
    val.erase(0, pos + 1);
    // The order is important. don't change.
    if (ADAFS_DATA->atime_state()) {
        pos = val.find(delim);
        attr.st_atim.tv_sec = static_cast<time_t>(stol(val.substr(0, pos)));
        val.erase(0, pos + 1);
    }
    if (ADAFS_DATA->mtime_state()) {
        pos = val.find(delim);
        attr.st_mtim.tv_sec = static_cast<time_t>(stol(val.substr(0, pos)));
        val.erase(0, pos + 1);
    }
    if (ADAFS_DATA->ctime_state()) {
        pos = val.find(delim);
        attr.st_ctim.tv_sec = static_cast<time_t>(stol(val.substr(0, pos)));
        val.erase(0, pos + 1);
    }
    if (ADAFS_DATA->uid_state()) {
        pos = val.find(delim);
        attr.st_uid = static_cast<uid_t>(stoul(val.substr(0, pos)));
        val.erase(0, pos + 1);
    }
    if (ADAFS_DATA->gid_state()) {
        pos = val.find(delim);
        attr.st_gid = static_cast<uid_t>(stoul(val.substr(0, pos)));
        val.erase(0, pos + 1);
    }
    if (ADAFS_DATA->inode_no_state()) {
        pos = val.find(delim);
        attr.st_ino = static_cast<ino_t>(stoul(val.substr(0, pos)));
        val.erase(0, pos + 1);
    }
    if (ADAFS_DATA->link_cnt_state()) {
        pos = val.find(delim);
        attr.st_nlink = static_cast<nlink_t>(stoul(val.substr(0, pos)));
        val.erase(0, pos + 1);
    }
    if (ADAFS_DATA->blocks_state()) { // last one will not encounter a delimter anymore
        attr.st_blocks = static_cast<blkcnt_t>(stoul(val));
    }
    return 0;
}

int get_attr(const std::string& path, struct stat* attr) {

    db_metadentry_to_stat(path, *attr);
    return 0;
}