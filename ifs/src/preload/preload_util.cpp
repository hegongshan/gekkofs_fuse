//
// Created by evie on 9/4/17.
//

#include <preload/preload_util.hpp>
#include <cstring>
#include <preload/preload.hpp>

static const std::string dentry_val_delim = ","s;

bool is_fs_path(const char* path) {
    return strstr(path, fs_config->mountdir.c_str()) == path;
}

/**
 * Converts the dentry db value into a stat struct, which is needed by Linux
 * @param path
 * @param db_val
 * @param attr
 * @return
 */
int db_val_to_stat(const std::string path, std::string db_val, struct stat& attr) {

    auto pos = db_val.find(dentry_val_delim);
    if (pos == std::string::npos) { // no delimiter found => no metadata enabled. fill with dummy values
        attr.st_ino = std::hash<std::string>{}(path);
        attr.st_mode = static_cast<unsigned int>(stoul(db_val));
        attr.st_nlink = 1;
        attr.st_uid = fs_config->uid;
        attr.st_gid = fs_config->gid;
        attr.st_size = 0;
        attr.st_blksize = BLOCKSIZE;
        attr.st_blocks = 0;
        attr.st_atim.tv_sec = 0;
        attr.st_mtim.tv_sec = 0;
        attr.st_ctim.tv_sec = 0;
        return 0;
    }
    // some metadata is enabled
    attr.st_mode = static_cast<unsigned int>(stoul(db_val.substr(0, pos)));
    db_val.erase(0, pos + 1);
    // The order is important. don't change.
    if (fs_config->atime_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_atim.tv_sec = static_cast<time_t>(stol(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->mtime_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_mtim.tv_sec = static_cast<time_t>(stol(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->ctime_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_ctim.tv_sec = static_cast<time_t>(stol(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->uid_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_uid = static_cast<uid_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->gid_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_gid = static_cast<uid_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->inode_no_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_ino = static_cast<ino_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->link_cnt_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_nlink = static_cast<nlink_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->blocks_state) { // last one will not encounter a delimiter anymore
        attr.st_blocks = static_cast<blkcnt_t>(stoul(db_val));
    }
    return 0;
}