//
// Created by evie on 5/9/17.
//

#ifndef LFS_DEnTRY_H
#define LFS_DEnTRY_H

#include "../main.h"

class Dentry {

private:
    std::string name_;
    fuse_ino_t inode_;
    mode_t mode_; // file type code (6 bits) + permission bits (9 bits rwx(user)rwx(group)rwx(others)

public:
    Dentry();

    Dentry(const std::string& name_);

    Dentry(const std::string& name_, fuse_ino_t inode_, mode_t mode_);

    const std::string& name() const;

    void name(const std::string& name_);

    fuse_ino_t inode() const;

    void inode(fuse_ino_t inode_);

    mode_t mode() const;

    void mode(mode_t mode_);


};


#endif //LFS_DEnTRY_H
