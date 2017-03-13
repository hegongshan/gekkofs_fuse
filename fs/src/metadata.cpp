//
// Created by draze on 3/5/17.
//

#include "metadata.h"

time_t Metadata::getAtime() const {
    return atime_;
}

void Metadata::setAtime(time_t atime) {
    Metadata::atime_ = atime;
}

time_t Metadata::getMtime() const {
    return mtime_;
}

void Metadata::setMtime(time_t mtime) {
    Metadata::mtime_ = mtime;
}

time_t Metadata::getCtime() const {
    return ctime_;
}

void Metadata::setCtime(time_t ctime_) {
    Metadata::ctime_ = ctime_;
}

void Metadata::setCtime(uint32_t ctime) {
    Metadata::ctime_ = ctime;
}

uint32_t Metadata::getUid() const {
    return uid_;
}

void Metadata::setUid(uint32_t uid) {
    Metadata::uid_ = uid;
}

uint32_t Metadata::getGid() const {
    return gid_;
}

void Metadata::setGid(uint32_t gid) {
    Metadata::gid_ = gid;
}

uint32_t Metadata::getMode() const {
    return mode_;
}

void Metadata::setMode(uint32_t mode) {
    Metadata::mode_ = mode;
}

uint64_t Metadata::getInode_no() const {
    return inode_no_;
}

void Metadata::setInode_no(uint64_t inode_no) {
    Metadata::inode_no_ = inode_no;
}

uint32_t Metadata::getLink_count() const {
    return link_count_;
}

void Metadata::setLink_count(uint32_t link_count) {
    Metadata::link_count_ = link_count;
}

uint32_t Metadata::getSize() const {
    return size_;
}

void Metadata::setSize(uint32_t size) {
    Metadata::size_ = size;
}

uint32_t Metadata::getBlocks() const {
    return blocks_;
}

void Metadata::setBlocks(uint32_t blocks) {
    Metadata::blocks_ = blocks;
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
                       inode_no_(),
                       link_count_(),
                       size_(),
                       blocks_() {}

Metadata::Metadata(mode_t mode) :
        atime_(),
        mtime_(),
        ctime_(),
        uid_(fuse_get_context()->uid),
        gid_(fuse_get_context()->gid),
        mode_(mode),
        inode_no_(),
        link_count_(0),
        size_(0),
        blocks_(0) {
    InitAcmTime();
    inode_no_ = util::GenerateInodeNo();
}

void Metadata::InitAcmTime(void) {
    std::time_t time;
    std::time(&time);
    atime_ = time;
    mtime_ = time;
    ctime_ = time;
}



