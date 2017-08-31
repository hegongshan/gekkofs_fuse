//
// Created by evie on 8/25/17.
//

#include "preload/open_file_map.hpp"

using namespace std;

OpenFile::OpenFile(const char* path, const int fd) : path_(path), fd_(fd)  {}

const char* OpenFile::path() const {
    return path_;
}

void OpenFile::path(const char* path_) {
    OpenFile::path_ = path_;
}

int OpenFile::fd() const {
    return fd_;
}

void OpenFile::fd(int fd_) {
    OpenFile::fd_ = fd_;
}

OpenFileMap::OpenFileMap() {}

OpenFile* OpenFileMap::get(int fd) {
    lock_guard<mutex> lock(files_mutex_);
    auto f = files_.find(fd);
    if (f == files_.end()) {
        return nullptr;
    } else {
        return &f->second;
    }
}

bool OpenFileMap::exist(const int fd) {
    lock_guard<mutex> lock(files_mutex_);
    auto f = files_.find(fd);
    return !(f == files_.end());
}

bool OpenFileMap::add(const char* path, const int fd) {
    OpenFile file{path, fd};
    lock_guard<mutex> lock(files_mutex_);
    files_.insert(make_pair(fd, file));
    return true;
}

bool OpenFileMap::remove(const int fd) {
    lock_guard<mutex> lock(files_mutex_);
    auto f = files_.find(fd);
    if (f == files_.end()) {
        return false;
    }
    files_.erase(fd);
    return true;
}

