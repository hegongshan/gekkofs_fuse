
#include <preload/open_file_map.hpp>
#include <preload/preload_util.hpp>

using namespace std;

OpenFile::OpenFile(const string& path, const int flags) : path_(path) {
    // set flags to OpenFile
    if (flags & O_CREAT)
        flags_[to_underlying(OpenFile_flags::creat)] = true;
    if (flags & O_APPEND)
        flags_[to_underlying(OpenFile_flags::append)] = true;
    if (flags & O_TRUNC)
        flags_[to_underlying(OpenFile_flags::trunc)] = true;
    if (flags & O_RDONLY)
        flags_[to_underlying(OpenFile_flags::rdonly)] = true;
    if (flags & O_WRONLY)
        flags_[to_underlying(OpenFile_flags::wronly)] = true;
    if (flags & O_RDWR)
        flags_[to_underlying(OpenFile_flags::rdwr)] = true;

    pos_ = 0; // If O_APPEND flag is used, it will be used before each write.
}

OpenFileMap::OpenFileMap() {}

OpenFile::~OpenFile() {

}

string OpenFile::path() const {
    return path_;
}

void OpenFile::path(const string& path_) {
    OpenFile::path_ = path_;
}

off64_t OpenFile::pos() {
    lock_guard<mutex> lock(pos_mutex_);
    return pos_;
}

void OpenFile::pos(off64_t pos_) {
    lock_guard<mutex> lock(pos_mutex_);
    OpenFile::pos_ = pos_;
}

const bool OpenFile::get_flag(OpenFile_flags flag) {
    lock_guard<mutex> lock(pos_mutex_);
    return flags_[to_underlying(flag)];
}

void OpenFile::set_flag(OpenFile_flags flag, bool value) {
    lock_guard<mutex> lock(flag_mutex_);
    flags_[to_underlying(flag)] = value;
}

// OpenFileMap starts here

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

int OpenFileMap::add(string path, const int flags) {
    auto fd = generate_fd_idx();
    /*
     * Check if fd is still in use and generate another if yes
     * Note that this can only happen once the all fd indices within the int has been used to the int::max
     * Once this limit is exceeded, we set fd_idx back to 3 and begin anew. Only then, if a file was open for
     * a long time will we have to generate another index.
     *
     * This situation can only occur when all fd indices have been given away once and we start again,
     * in which case the fd_validation_needed flag is set. fd_validation is set to false, if
     */
    if (fd_validation_needed) {
        while (exist(fd)) {
            fd = generate_fd_idx();
        }
    }
    auto open_file = make_shared<OpenFile>(path, flags);
    lock_guard<mutex> lock(files_mutex_);
    files_.insert(make_pair(fd, open_file));
    return fd;
}

bool OpenFileMap::remove(const int fd) {
    lock_guard<mutex> lock(files_mutex_);
    auto f = files_.find(fd);
    if (f == files_.end()) {
        return false;
    }
    files_.erase(fd);
    if (fd_validation_needed && files_.empty()) {
        fd_validation_needed = false;
        ld_logger->info("{}() fd_validation flag reset", __func__);
    }
    return true;
}

