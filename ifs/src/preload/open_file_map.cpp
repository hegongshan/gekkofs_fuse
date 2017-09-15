//
// Created by evie on 8/25/17.
//

#include <preload/open_file_map.hpp>

using namespace std;

OpenFile::OpenFile(const char* path, const bool append_flag) : path_(path), append_flag_(append_flag) {
    tmp_file_ = tmpfile(); // create a temporary file in memory and
    fd_ = fileno(tmp_file_); // get a valid file descriptor from the kernel
}

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

OpenFile::~OpenFile() {
    if (tmp_file_ != nullptr)
        fclose(tmp_file_);
}

void OpenFile::annul_fd() {
    if (tmp_file_ != nullptr)
        fclose(tmp_file_);
}

bool OpenFile::append_flag() const {
    return append_flag_;
}

void OpenFile::append_flag(bool append_flag) {
    OpenFile::append_flag_ = append_flag;
}

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

int OpenFileMap::add(const char* path, const bool append) {
    OpenFile file{path, append};
    lock_guard<mutex> lock(files_mutex_);
    files_.insert(make_pair(file.fd(), file));
    return file.fd();
}

bool OpenFileMap::remove(const int fd) {
    lock_guard<mutex> lock(files_mutex_);
    auto f = files_.find(fd);
    if (f == files_.end()) {
        return false;
    }
    files_.at(fd).annul_fd(); // free file descriptor
    files_.erase(fd);
    return true;
}

