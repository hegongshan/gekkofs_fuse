
#ifndef IFS_OPEN_FILE_MAP_HPP
#define IFS_OPEN_FILE_MAP_HPP

#include <map>
#include <mutex>
#include <memory>

enum class OpenFile_flags {
    append = 0,
    creat,
    trunc,
    rdonly,
    wronly,
    rdwr,
    flag_count // this is purely used as a size variable of this enum class
};

class OpenFile {
private:
    std::string path_;
    std::array<bool, static_cast<int>(OpenFile_flags::flag_count)> flags_ = {false};
    off_t pos_;
    std::mutex pos_mutex_;
    std::mutex flag_mutex_;

public:
    // multiple threads may want to update the file position if fd has been duplicated by dup()

    OpenFile(const std::string& path, int flags);

    ~OpenFile();

    // getter/setter
    std::string path() const;

    void path(const std::string& path_);

    off_t pos();

    void pos(off_t pos_);

    const bool get_flag(OpenFile_flags flag);

    void set_flag(OpenFile_flags flag, bool value);

};


class OpenFileMap {

private:
    std::map<int, std::shared_ptr<OpenFile>> files_;
    std::mutex files_mutex_;


public:
    OpenFileMap();

    OpenFile* get(int fd);

    bool exist(int fd);

    int add(std::string path, int flags);

    bool remove(int fd);

};


#endif //IFS_OPEN_FILE_MAP_HPP
