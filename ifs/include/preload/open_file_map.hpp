//
// Created by evie on 8/25/17.
//

#ifndef IFS_OPEN_FILE_MAP_HPP
#define IFS_OPEN_FILE_MAP_HPP

#include <map>
#include <mutex>

class OpenFile {
private:
    const char* path_;
    int fd_;

public:
    OpenFile(const char* path, const int fd);

    // getter/setter
    const char* path() const;

    void path(const char* path_);

    int fd() const;

    void fd(int fd_);
};


class OpenFileMap {

private:
    typedef std::map<int, OpenFile> FileMap;
    FileMap files_;
    std::mutex files_mutex_;


public:
    OpenFileMap();

    OpenFile* get(int fd);
    bool exist(const int fd);
    bool add(const char* path, const int fd);
    bool remove(const int fd);

};


#endif //IFS_OPEN_FILE_MAP_HPP
