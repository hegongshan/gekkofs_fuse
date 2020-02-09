/*
  Copyright 2018-2019, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2019, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#ifndef GEKKOFS_GKFS_FUNCTIONS_HPP
#define GEKKOFS_GKFS_FUNCTIONS_HPP

#include <client/open_file_map.hpp>
#include <global/metadata.hpp>

struct linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[1];
};

struct linux_dirent64 {
    unsigned long long d_ino;
    unsigned long long d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[1];
};

using sys_statfs = struct statfs;
using sys_statvfs = struct statvfs;

namespace gkfs {
    namespace func {


        std::shared_ptr<Metadata> metadata(const std::string& path, bool follow_links = false);

        int check_parent_dir(const std::string& path);

        int open(const std::string& path, mode_t mode, int flags);

        int mk_node(const std::string& path, mode_t mode);

        int rm_node(const std::string& path);

        int access(const std::string& path, int mask, bool follow_links = true);

        int stat(const std::string& path, struct stat* buf, bool follow_links = true);

        int statfs(sys_statfs* buf);

        int statvfs(sys_statvfs* buf);

        off64_t lseek(unsigned int fd, off64_t offset, unsigned int whence);

        off64_t lseek(std::shared_ptr<OpenFile> gkfs_fd, off64_t offset, unsigned int whence);

        int truncate(const std::string& path, off_t offset);

        int truncate(const std::string& path, off_t old_size, off_t new_size);

        int dup(int oldfd);

        int dup2(int oldfd, int newfd);

#ifdef HAS_SYMLINKS

        int mk_symlink(const std::string& path, const std::string& target_path);

        int readlink(const std::string& path, char* buf, int bufsize);

#endif

        ssize_t pwrite(std::shared_ptr<OpenFile> file,
                       const char* buf, size_t count, off64_t offset);

        ssize_t pwrite_ws(int fd, const void* buf, size_t count, off64_t offset);

        ssize_t write(int fd, const void* buf, size_t count);

        ssize_t pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset);

        ssize_t writev(int fd, const struct iovec* iov, int iovcnt);

        ssize_t pread(std::shared_ptr<OpenFile> file, char* buf, size_t count, off64_t offset);

        ssize_t pread_ws(int fd, void* buf, size_t count, off64_t offset);

        ssize_t read(int fd, void* buf, size_t count);


        int opendir(const std::string& path);

        int getdents(unsigned int fd,
                     struct linux_dirent* dirp,
                     unsigned int count);

        int getdents64(unsigned int fd,
                       struct linux_dirent64* dirp,
                       unsigned int count);

        int rmdir(const std::string& path);
    }
}

#endif //GEKKOFS_GKFS_FUNCTIONS_HPP
