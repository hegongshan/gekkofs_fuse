/**
 * All intercepted functions are defined here
 */
#include <preload/preload.hpp>
#include <preload/passthrough.hpp>
#include <preload/adafs_functions.hpp>

using namespace std;

OpenFileMap file_map{};

int open(const char* path, int flags, ...) {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list vl;
        va_start(vl, flags);
        mode = static_cast<mode_t>(va_arg(vl, int));
        va_end(vl);
    }
    if (ld_is_aux_loaded() && is_fs_path(path)) {
        return adafs_open(path, mode, flags);
    }
    return (reinterpret_cast<decltype(&open)>(libc_open))(path, flags, mode);
}

int open64(__const char* path, int flags, ...) {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return open(path, flags | O_LARGEFILE, mode);
}

//// TODO This function somehow always blocks forever if one puts anything between the paththru...
//FILE* fopen(const char* path, const char* mode) {
////    init_passthrough_if_needed();
////    DAEMON_DEBUG(debug_fd, "fopen called with path %s\n", path);
//    return (reinterpret_cast<decltype(&fopen)>(libc_fopen))(path, mode);
//}

//// TODO This function somehow always blocks forever if one puts anything between the paththru...
//FILE* fopen64(const char* path, const char* mode) {
////    init_passthrough_if_needed();
////    DAEMON_DEBUG(debug_fd, "fopen64 called with path %s\n", path);
//    return (reinterpret_cast<decltype(&fopen)>(libc_fopen))(path, mode);
//}

#undef creat

int creat(const char* path, mode_t mode) {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {} with mode {}", __func__, path, mode);
    return open(path, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

#undef creat64

int creat64(const char* path, mode_t mode) {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {} with mode {}", __func__, path, mode);
    return open(path, O_CREAT | O_WRONLY | O_TRUNC | O_LARGEFILE, mode);
}

int mkdir(const char* path, mode_t mode) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {} with mode {}", __func__, path, mode);
    if (ld_is_aux_loaded() && is_fs_path(path)) {
        return adafs_mk_node(path, mode);
    }
    return (reinterpret_cast<decltype(&mkdir)>(libc_mkdir))(path, mode);
}

int mkdirat(int dirfd, const char* path, mode_t mode) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {} with mode {} with dirfd {}", __func__, path, mode, dirfd);
    if (ld_is_aux_loaded() && is_fs_path(path)) {
        // not implemented
        ld_logger->trace("{}() not implemented.", __func__);
        return -1;
    }
    return (reinterpret_cast<decltype(&mkdirat)>(libc_mkdirat))(dirfd, path, mode);
}


int unlink(const char* path) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    if (ld_is_aux_loaded() && is_fs_path(path)) {
        return adafs_rm_node(path);
    }
    return (reinterpret_cast<decltype(&unlink)>(libc_unlink))(path);
}

int rmdir(const char* path) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    if (ld_is_aux_loaded() && is_fs_path(path)) {
        // XXX Possible need another call to specifically handle remove dirs. For now handle them the same as files
        return adafs_rm_node(path);
    }
    return (reinterpret_cast<decltype(&rmdir)>(libc_rmdir))(path);
}

int close(int fd) {
    init_passthrough_if_needed();
    if (ld_is_aux_loaded() && file_map.exist(fd)) {
        // Currently no call to the daemon is required
        file_map.remove(fd);
        return 0;
    }
    return (reinterpret_cast<decltype(&close)>(libc_close))(fd);
}

int __close(int fd) {
    return close(fd);
}

int access(const char* path, int mask) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called path {} mask {}", __func__, path, mask);
    if (ld_is_aux_loaded() && is_fs_path(path)) {
        return adafs_access(path, mask);
    }
    return (reinterpret_cast<decltype(&access)>(libc_access))(path, mask);
}

int faccessat(int dirfd, const char* path, int mode, int flags) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called path {} mode {} dirfd {} flags {}", __func__, path, mode, dirfd, flags);
    if (ld_is_aux_loaded() && is_fs_path(path)) {
        // not implemented
        ld_logger->trace("{}() not implemented.", __func__);
        return -1;
    }
    return (reinterpret_cast<decltype(&faccessat)>(libc_faccessat))(dirfd, path, mode, flags);
}


int stat(const char* path, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    if (ld_is_aux_loaded() && is_fs_path(path)) {
        return adafs_stat(path, buf);
    }
    return (reinterpret_cast<decltype(&stat)>(libc_stat))(path, buf);
}

int fstat(int fd, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with fd {}", __func__, fd);
    if (ld_is_aux_loaded() && file_map.exist(fd)) {
        auto path = file_map.get(fd)->path();
        return adafs_stat(path, buf);
    }
    return (reinterpret_cast<decltype(&fstat)>(libc_fstat))(fd, buf);
}

int __xstat(int ver, const char* path, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    if (ld_is_aux_loaded() && is_fs_path(path)) {
        return adafs_stat(path, buf);
    }
    return (reinterpret_cast<decltype(&__xstat)>(libc___xstat))(ver, path, buf);
}

int __xstat64(int ver, const char* path, struct stat64* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    if (ld_is_aux_loaded() && is_fs_path(path)) {
        return adafs_stat64(path, buf);
//        // Not implemented
//        return -1;
    }
    return (reinterpret_cast<decltype(&__xstat64)>(libc___xstat64))(ver, path, buf);
}

int __fxstat(int ver, int fd, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with fd {}", __func__, fd);
    if (ld_is_aux_loaded() && file_map.exist(fd)) {
        auto path = file_map.get(fd)->path();
        return adafs_stat(path, buf);
    }
    return (reinterpret_cast<decltype(&__fxstat)>(libc___fxstat))(ver, fd, buf);
}

int __fxstat64(int ver, int fd, struct stat64* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with fd {}", __func__, fd);
    if (ld_is_aux_loaded() && file_map.exist(fd)) {
        auto path = file_map.get(fd)->path();
        return adafs_stat64(path, buf);
    }
    return (reinterpret_cast<decltype(&__fxstat64)>(libc___fxstat64))(ver, fd, buf);
}

extern int __lxstat(int ver, const char* path, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    if (ld_is_aux_loaded() && is_fs_path(path)) {
// Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&__lxstat)>(libc___lxstat))(ver, path, buf);
}

extern int __lxstat64(int ver, const char* path, struct stat64* buf) __THROW {
    init_passthrough_if_needed();
    if (ld_is_aux_loaded() && is_fs_path(path)) {
// Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&__lxstat64)>(libc___lxstat64))(ver, path, buf);
}

int puts(const char* str) {
    return (reinterpret_cast<decltype(&puts)>(libc_puts))(str);
}

ssize_t write(int fd, const void* buf, size_t count) {
    init_passthrough_if_needed();
    if (ld_is_aux_loaded() && file_map.exist(fd)) {
        auto adafs_fd = file_map.get(fd);
        auto pos = adafs_fd->pos(); // retrieve the current offset
        ld_logger->trace("{}() called with fd {}", __func__, fd);
        // TODO if append flag has been given, set offset accordingly.
        // XXX handle lseek too
        auto ret = adafs_pwrite_ws(fd, buf, count, pos);
        if(ret!=-1){
            adafs_fd->pos(pos + (off_t) count);
        }
        return ret;
    }
    return (reinterpret_cast<decltype(&write)>(libc_write))(fd, buf, count);
}

ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset) {
    init_passthrough_if_needed();
    if (ld_is_aux_loaded() && file_map.exist(fd)) {
        ld_logger->trace("{}() called with fd {}", __func__, fd);
        return adafs_pwrite_ws(fd, buf, count, offset);
    }
    return (reinterpret_cast<decltype(&pwrite)>(libc_pwrite))(fd, buf, count, offset);
}

ssize_t pwrite64(int fd, const void* buf, size_t count, __off64_t offset) {
    init_passthrough_if_needed();
    if (ld_is_aux_loaded() && file_map.exist(fd)) {
        ld_logger->trace("{}() called with fd {}", __func__, fd);
        return adafs_pwrite_ws(fd, buf, count, offset);
    }
    return (reinterpret_cast<decltype(&pwrite64)>(libc_pwrite64))(fd, buf, count, offset);
}

ssize_t read(int fd, void* buf, size_t count) {
    init_passthrough_if_needed();
    if (ld_is_aux_loaded() && file_map.exist(fd)) {
        auto adafs_fd = file_map.get(fd);
        auto pos = adafs_fd->pos(); //retrieve the current offset
        ld_logger->trace("{}() called with fd {}", __func__, fd);
        auto ret= adafs_pread_ws(fd, buf, count, pos);
        //Update offset
        if(ret!=-1){
           adafs_fd->pos(pos + (off_t) count);
        }
        return ret;
    }
    return (reinterpret_cast<decltype(&read)>(libc_read))(fd, buf, count);
}

ssize_t pread(int fd, void* buf, size_t count, off_t offset) {
    init_passthrough_if_needed();
    if (ld_is_aux_loaded() && file_map.exist(fd)) {
        ld_logger->trace("{}() called with fd {}", __func__, fd);
        return adafs_pread_ws(fd, buf, count, offset);
    }
    return (reinterpret_cast<decltype(&pread)>(libc_pread))(fd, buf, count, offset);
}

ssize_t pread64(int fd, void* buf, size_t count, __off64_t offset) {
    init_passthrough_if_needed();
    if (ld_is_aux_loaded() && file_map.exist(fd)) {
        ld_logger->trace("{}() called with fd {}", __func__, fd);
        return adafs_pread_ws(fd, buf, count, offset);
    }
    return (reinterpret_cast<decltype(&pread64)>(libc_pread64))(fd, buf, count, offset);
}

off_t lseek(int fd, off_t offset, int whence) __THROW {
    init_passthrough_if_needed();
    return (reinterpret_cast<decltype(&lseek)>(libc_lseek))(fd, offset, whence);
}

off_t lseek64(int fd, off_t offset, int whence) __THROW {
    init_passthrough_if_needed();
    return lseek(fd, offset, whence);
}

int truncate(const char* path, off_t length) __THROW {
    init_passthrough_if_needed();
    return (reinterpret_cast<decltype(&truncate)>(libc_truncate))(path, length);
}

int ftruncate(int fd, off_t length) __THROW {
    init_passthrough_if_needed();
    return (reinterpret_cast<decltype(&ftruncate)>(libc_ftruncate))(fd, length);
}

int dup(int oldfd) __THROW {
    init_passthrough_if_needed();
    if (ld_is_aux_loaded() && file_map.exist(oldfd)) {
// Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&dup)>(libc_dup))(oldfd);
}

int dup2(int oldfd, int newfd) __THROW {
    init_passthrough_if_needed();
    if (ld_is_aux_loaded() && (file_map.exist(oldfd) || file_map.exist(newfd))) {
// Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&dup2)>(libc_dup2))(oldfd, newfd);
}