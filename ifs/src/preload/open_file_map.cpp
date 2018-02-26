
#include <preload/open_file_map.hpp>
#include <preload/preload_util.hpp>

using namespace std;

OpenFile::OpenFile(const string& path, const bool append_flag) : path_(path), append_flag_(append_flag) {
    tmp_file_ = tmpfile(); // create a temporary file in memory and
    if (tmp_file_ == NULL) {
        ld_logger->error("{}() While creating temporary file in OpenFile constructor {}", __func__, strerror(errno));
        cout << strerror(errno) << endl;
        exit(1);
    }
    fd_ = fileno(tmp_file_); // get a valid file descriptor from the kernel
    pos_ = 0; // TODO set the pos according to the flag and delete the flag from fd map??
}

string OpenFile::path() const {
    return path_;
}

void OpenFile::path(const string& path_) {
    OpenFile::path_ = path_;
}

int OpenFile::fd() const {
    return fd_;
}

void OpenFile::fd(int fd_) {
    OpenFile::fd_ = fd_;
}

off_t OpenFile::pos() const {
    return pos_;
}

void OpenFile::pos(off_t pos_) {
    OpenFile::pos_=pos_;
}

OpenFileMap::OpenFileMap() {}

OpenFile::~OpenFile() {
//    if (tmp_file_ != nullptr) // XXX This crashes when preload lib is shut down. annul_fd should always be called!
//        fclose(tmp_file_);
}

void OpenFile::annul_fd() {
    if (tmp_file_ != nullptr) {
        if (fclose(tmp_file_) != 0)
            ld_logger->error("{}() Unable to close tmp fd for path {}", __func__, this->path_);
    }
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
        return f->second.get();
    }
}

bool OpenFileMap::exist(const int fd) {
    lock_guard<mutex> lock(files_mutex_);
    auto f = files_.find(fd);
    return !(f == files_.end());
}

int OpenFileMap::add(string path, const bool append) {
    auto file = make_shared<OpenFile>(path, append);
    lock_guard<mutex> lock(files_mutex_);
    files_.insert(make_pair(file->fd(), file));
    return file->fd();
}

bool OpenFileMap::remove(const int fd) {
    lock_guard<mutex> lock(files_mutex_);
    auto f = files_.find(fd);
    if (f == files_.end()) {
        return false;
    }
    files_.at(fd)->annul_fd(); // free file descriptor
    files_.erase(fd);
    return true;
}

