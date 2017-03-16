
#ifndef FS_METADATA_H
#define FS_METADATA_H

#include "main.h"

//TODO we might want to replace this with GOOGLE PROTOBUF
class Metadata {

private:
    time_t atime_;             // access time. gets updated on file access unless mounted with noatime
    time_t mtime_;             // modify time. gets updated when file content is modified.
    time_t ctime_;             // change time. gets updated when the file attributes are changed AND when file content is modified.
    uint32_t uid_;
    uint32_t gid_;
    uint32_t mode_;
    uint64_t inode_no_;
    uint32_t link_count_;        // number of names for this inode (hardlinks)
    uint32_t size_;              // size_ in bytes, might be computed instead of stored
    uint32_t blocks_;            // allocated file system blocks_


public:
    Metadata();

    Metadata(mode_t mode);

    void init_ACM_time();

    //Getter and Setter
    time_t atime() const;

    void atime(time_t atime_);

    time_t mtime() const;

    void mtime(time_t mtime_);

    time_t ctime() const;

    void ctime(time_t ctime_);

    uint32_t uid() const;

    void uid(uint32_t uid_);

    uint32_t gid() const;

    void gid(uint32_t gid_);

    uint32_t mode() const;

    void mode(uint32_t mode_);

    uint64_t inode_no() const;

    void inode_no(uint64_t inode_no_);

    uint32_t link_count() const;

    void link_count(uint32_t link_count_);

    uint32_t size() const;

    void size(uint32_t size_);

    uint32_t blocks() const;

    void blocks(uint32_t blocks_);

};


#endif //FS_METADATA_H
