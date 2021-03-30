#ifndef GEKKOFS_OPEN_DIR_HPP
#define GEKKOFS_OPEN_DIR_HPP

#include <string>
#include <vector>

#include <client/open_file_map.hpp>

namespace gkfs::filemap {

class DirEntry {
private:
    std::string name_;
    FileType type_;

public:
    DirEntry(const std::string& name, FileType type);

    const std::string&
    name();

    FileType
    type();
};

class OpenDir : public OpenFile {
private:
    std::vector<DirEntry> entries;


public:
    explicit OpenDir(const std::string& path);

    void
    add(const std::string& name, const FileType& type);

    const DirEntry&
    getdent(unsigned int pos);

    size_t
    size();
};

} // namespace gkfs::filemap

#endif // GEKKOFS_OPEN_DIR_HPP
