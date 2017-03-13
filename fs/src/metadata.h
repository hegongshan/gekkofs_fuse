
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

    void InitAcmTime(void);

public:
    Metadata();

    Metadata(mode_t mode);

    //Getter and Setter
    time_t getAtime() const;

    void setAtime(time_t atime);

    time_t getMtime() const;

    void setMtime(time_t mtime);

    time_t getCtime() const;

    void setCtime(time_t ctime_);

    void setCtime(uint32_t ctime);

    uint32_t getUid() const;

    void setUid(uint32_t uid);

    uint32_t getGid() const;

    void setGid(uint32_t gid);

    uint32_t getMode() const;

    void setMode(uint32_t mode);

    uint64_t getInode_no() const;

    void setInode_no(uint64_t inode_no);

    uint32_t getLink_count() const;

    void setLink_count(uint32_t link_count);

    uint32_t getSize() const;

    void setSize(uint32_t size);

    uint32_t getBlocks() const;

    void setBlocks(uint32_t blocks);

};


#endif //FS_METADATA_H
