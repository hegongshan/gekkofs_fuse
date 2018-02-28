#ifndef IFS_ADAFS_FUNCTIONS_HPP
#define IFS_ADAFS_FUNCTIONS_HPP

#include <preload/preload_util.hpp>

int adafs_open(const std::string& path, mode_t mode, int flags);

int adafs_mk_node(const std::string& path, const mode_t mode);

int adafs_rm_node(const std::string& path);

int adafs_access(const std::string& path, const int mask);

int adafs_stat(const std::string& path, struct stat* buf);

int adafs_stat64(const std::string& path, struct stat64* buf);

off_t adafs_lseek(int fd, off_t offset, int whence);

off_t adafs_lseek(OpenFile* adafs_fd, off_t offset, int whence);

ssize_t adafs_pread_ws(int fd, void* buf, size_t count, off_t offset);

ssize_t adafs_pwrite_ws(int fd, const void* buf, size_t count, off_t offset);


#endif //IFS_ADAFS_FUNCTIONS_HPP
