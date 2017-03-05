//
// Created by draze on 3/5/17.
//

#include "metadata.h"

time_t Metadata::getAtime_() const {
    return atime_;
}

void Metadata::setAtime_(time_t atime_) {
    Metadata::atime_ = atime_;
}

time_t Metadata::getMtime_() const {
    return mtime_;
}

void Metadata::setMtime_(time_t mtime_) {
    Metadata::mtime_ = mtime_;
}

time_t Metadata::getCtime_() const {
    return ctime_;
}

void Metadata::setCtime_(time_t ctime_) {
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
Metadata::Metadata() {

}

Metadata::Metadata(mode_t mode) { //TODO add initializer list
    init_ACMtime();
    uid_ = fuse_get_context()->uid;
    gid_ = fuse_get_context()->gid;
    mode_ = mode;
    inode_no_ = util::generate_inode_no();
    link_count_ = 0;
    size_ = 0;
    blocks_ = 0;
}

void Metadata::init_ACMtime(void) {
    std::time_t time;
    std::time(&time);
    atime_ = time;
    mtime_ = time;
    ctime_ = time;
}



