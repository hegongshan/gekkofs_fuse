
#ifndef IFS_OPEN_FILE_MAP_HPP
#define IFS_OPEN_FILE_MAP_HPP

#include <map>
#include <mutex>
#include <memory>

class OpenFile {
private:
    std::string path_;
    bool append_flag_;

    int fd_;
    // XXX add mutex for pos. If dup is used excessively pos_ may be updated concurrently. The mutex is implemented in pos setter
    off_t pos_;
    FILE* tmp_file_;
    /*
XXX
shared:
- path
- flags
- pos

unique:
- sys fd (throw out. is already part as the key in the filemap)
- tmp_file


- int fd points to same shared ptr in file map
- fd attribute is not used -> throw out
- put tmp_file into another map<int(fd), FILE*>
- Add mutex for pos_
- Let's also properly add all flags from open in there into an enum or similar



     */

public:
    OpenFile(const std::string& path, bool append_flag);

    ~OpenFile();

    void annul_fd();

    // getter/setter
    std::string path() const;

    void path(const std::string& path_);

    int fd() const;

    void fd(int fd_);

    off_t pos() const;

    void pos(off_t pos_);

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

    bool exist(int fd);

    int add(std::string path, bool append);

    bool remove(int fd);

};


#endif //IFS_OPEN_FILE_MAP_HPP
