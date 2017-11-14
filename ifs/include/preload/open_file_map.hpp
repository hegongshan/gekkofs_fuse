
#ifndef IFS_OPEN_FILE_MAP_HPP
#define IFS_OPEN_FILE_MAP_HPP

#include <preload/preload.hpp>

class OpenFile {
private:
    std::string path_;
    bool append_flag_;

    int fd_;
    FILE* tmp_file_;

public:
    OpenFile(const std::string& path, const bool append_flag);

    ~OpenFile();

    void annul_fd();

    // getter/setter
    std::string path() const;

    void path(const std::string& path_);

    int fd() const;

    void fd(int fd_);

    bool append_flag() const;

    void append_flag(bool append_flag);

};


class OpenFileMap {

private:
    std::map<int, std::shared_ptr<OpenFile>> files_;
    std::mutex files_mutex_;


public:
    OpenFileMap();

    OpenFile* get(int fd);
    bool exist(const int fd);

    int add(std::string path, const bool append);
    bool remove(const int fd);

};


#endif //IFS_OPEN_FILE_MAP_HPP
