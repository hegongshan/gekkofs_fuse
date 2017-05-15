//
// Created by draze on 3/5/17.
//

#include "metadata.h"

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

fuse_ino_t Metadata::inode_no() const {
    return inode_no_;
}

void Metadata::inode_no(fuse_ino_t inode_no_) {
    Metadata::inode_no_ = inode_no_;
}

nlink_t Metadata::link_count() const {
    return link_count_;
}

void Metadata::link_count(nlink_t link_count_) {
    Metadata::link_count_ = link_count_;
}

off_t Metadata::size() const {
    return size_;
}

void Metadata::size(off_t size_) {
    Metadata::size_ = size_;
}

blkcnt_t Metadata::blocks() const {
    return blocks_;
}

void Metadata::blocks(blkcnt_t blocks_) {
    Metadata::blocks_ = blocks_;
}

//--------------------------------------------
// By default create an empty metadata object
//Metadata::Metadata() : Metadata(S_IFREG | 0755) {}
Metadata::Metadata() : atime_(),
                       mtime_(),
                       ctime_(),
                       uid_(),
                       gid_(),
                       mode_(),
                       inode_no_(0),
                       link_count_(0),
                       size_(0),
                       blocks_(0) {}

Metadata::Metadata(mode_t mode, uint32_t uid, uint32_t gid, fuse_req_t& req) :
        atime_(),
        mtime_(),
        ctime_(),
        uid_(uid),
        gid_(gid),
        mode_(mode),
        inode_no_(0),
        link_count_(0),
        size_(0),
        blocks_(0) {
    init_ACM_time();
    inode_no_ = Util::generate_inode_no(req);
}

Metadata::Metadata(mode_t mode, uid_t uid, gid_t gid, fuse_ino_t inode) :
        atime_(),
        mtime_(),
        ctime_(),
        uid_(uid),
        gid_(gid),
        mode_(mode),
        inode_no_(inode),
        link_count_(0),
        size_(0),
        blocks_(0) {
    init_ACM_time();
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
