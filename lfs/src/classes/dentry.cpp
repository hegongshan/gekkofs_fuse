//
// Created by evie on 5/9/17.
//

#include "dentry.h"

Dentry::Dentry() {}

Dentry::Dentry(const std::string& name_) : name_(name_) {}

Dentry::Dentry(const std::string& name_, fuse_ino_t inode_, mode_t mode_) : name_(name_), inode_(inode_),
                                                                            mode_(mode_) {}

const std::string& Dentry::name() const {
    return name_;
}

void Dentry::name(const std::string& name_) {
    Dentry::name_ = name_;
}

fuse_ino_t Dentry::inode() const {
    return inode_;
}

void Dentry::inode(fuse_ino_t inode_) {
    Dentry::inode_ = inode_;
}

mode_t Dentry::mode() const {
    return mode_;
}

void Dentry::mode(mode_t mode_) {
    Dentry::mode_ = mode_;
}
