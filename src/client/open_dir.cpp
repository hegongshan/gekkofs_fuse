#include <client/open_dir.hpp>
#include <stdexcept>
#include <cstring>

namespace gkfs {
namespace filemap {

DirEntry::DirEntry(const std::string& name, const FileType type)
    : name_(name), type_(type) {}

const std::string&
DirEntry::name() {
    return name_;
}

FileType
DirEntry::type() {
    return type_;
}


OpenDir::OpenDir(const std::string& path)
    : OpenFile(path, 0, FileType::directory) {}


void
OpenDir::add(const std::string& name, const FileType& type) {
    entries.push_back(DirEntry(name, type));
}

const DirEntry&
OpenDir::getdent(unsigned int pos) {
    return entries.at(pos);
}

size_t
OpenDir::size() {
    return entries.size();
}

} // namespace filemap
} // namespace gkfs