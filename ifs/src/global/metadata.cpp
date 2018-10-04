#include "global/metadata.hpp"
#include "global/configure.hpp"

#include <fmt/fmt.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ctime>


static const char MSP = '|'; // metadata separator

Metadata::Metadata(const mode_t mode) :
    atime_(),
    mtime_(),
    ctime_(),
    uid_(),
    gid_(),
    mode_(mode),
    inode_no_(0),
    link_count_(0),
    size_(0),
    blocks_(0)
{}

Metadata::Metadata(std::string db_val) {
    auto pos = db_val.find(MSP);
    if (pos == std::string::npos) { // no delimiter found => no metadata enabled. fill with dummy values
        mode_ = static_cast<unsigned int>(stoul(db_val));
        link_count_ = 1;
        uid_ = 0;
        gid_ = 0;
        size_ = 0;
        blocks_ = 0;
        atime_ = 0;
        mtime_ = 0;
        ctime_ = 0;
        return;
    }
    // some metadata is enabled: mode is always there
    mode_ = static_cast<unsigned int>(stoul(db_val.substr(0, pos)));
    db_val.erase(0, pos + 1);
    // size is also there
    pos = db_val.find(MSP);
    if (pos != std::string::npos) {  // delimiter found. more metadata is coming
        size_ = stol(db_val.substr(0, pos));
        db_val.erase(0, pos + 1);
    } else {
        size_ = stol(db_val);
        return;
    }
    // The order is important. don't change.
    if (MDATA_USE_ATIME) {
        pos = db_val.find(MSP);
        atime_ = static_cast<time_t>(stol(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (MDATA_USE_MTIME) {
        pos = db_val.find(MSP);
        mtime_ = static_cast<time_t>(stol(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (MDATA_USE_CTIME) {
        pos = db_val.find(MSP);
        ctime_ = static_cast<time_t>(stol(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (MDATA_USE_UID) {
        pos = db_val.find(MSP);
        uid_ = static_cast<uid_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (MDATA_USE_GID) {
        pos = db_val.find(MSP);
        gid_ = static_cast<uid_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (MDATA_USE_INODE_NO) {
        pos = db_val.find(MSP);
        inode_no_ = static_cast<ino_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (MDATA_USE_LINK_CNT) {
        pos = db_val.find(MSP);
        link_count_ = static_cast<nlink_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (MDATA_USE_BLOCKS) { // last one will not encounter a delimiter anymore
        blocks_ = static_cast<blkcnt_t>(stoul(db_val));
    }
}

std::string Metadata::serialize() const
{
    std::string s;
    // The order is important. don't change.
    s += fmt::format_int(mode_).c_str(); // add mandatory mode
    s += MSP;
    s += fmt::format_int(size_).c_str(); // add mandatory size
    if (MDATA_USE_ATIME) {
        s += MSP;
        s += fmt::format_int(atime_).c_str();
    }
    if (MDATA_USE_MTIME) {
        s += MSP;
        s += fmt::format_int(mtime_).c_str();
    }
    if (MDATA_USE_CTIME) {
        s += MSP;
        s += fmt::format_int(ctime_).c_str();
    }
    if (MDATA_USE_UID) {
        s += MSP;
        s += fmt::format_int(uid_).c_str();
    }
    if (MDATA_USE_GID) {
        s += MSP;
        s += fmt::format_int(gid_).c_str();
    }
    if (MDATA_USE_INODE_NO) {
        s += MSP;
        s += fmt::format_int(inode_no_).c_str();
    }
    if (MDATA_USE_LINK_CNT) {
        s += MSP;
        s += fmt::format_int(link_count_).c_str();
    }
    if (MDATA_USE_BLOCKS) {
        s += MSP;
        s += fmt::format_int(blocks_).c_str();
    }
    return s;
}

void Metadata::init_ACM_time() {
    std::time_t time;
    std::time(&time);
    atime_ = time;
    mtime_ = time;
    ctime_ = time;
}

void Metadata::update_ACM_time(bool a, bool c, bool m) {
    std::time_t time;
    std::time(&time);
    if (a)
        atime_ = time;
    if (c)
        ctime_ = time;
    if (m)
        mtime_ = time;
}

//-------------------------------------------- GETTER/SETTER

time_t Metadata::atime() const {
    return atime_;
}

void Metadata::atime(time_t atime_) {
    Metadata::atime_ = atime_;
}

time_t Metadata::mtime() const {
    return mtime_;
}

void Metadata::mtime(time_t mtime_) {
    Metadata::mtime_ = mtime_;
}

time_t Metadata::ctime() const {
    return ctime_;
}

void Metadata::ctime(time_t ctime_) {
    Metadata::ctime_ = ctime_;
}

uid_t Metadata::uid() const {
    return uid_;
}

void Metadata::uid(uid_t uid_) {
    Metadata::uid_ = uid_;
}

gid_t Metadata::gid() const {
    return gid_;
}

void Metadata::gid(gid_t gid_) {
    Metadata::gid_ = gid_;
}

mode_t Metadata::mode() const {
    return mode_;
}

void Metadata::mode(mode_t mode_) {
    Metadata::mode_ = mode_;
}

uint64_t Metadata::inode_no() const {
    return inode_no_;
}

void Metadata::inode_no(uint64_t inode_no_) {
    Metadata::inode_no_ = inode_no_;
}

nlink_t Metadata::link_count() const {
    return link_count_;
}

void Metadata::link_count(nlink_t link_count_) {
    Metadata::link_count_ = link_count_;
}

size_t Metadata::size() const {
    return size_;
}

void Metadata::size(size_t size_) {
    Metadata::size_ = size_;
}

blkcnt_t Metadata::blocks() const {
    return blocks_;
}

void Metadata::blocks(blkcnt_t blocks_) {
    Metadata::blocks_ = blocks_;
}
