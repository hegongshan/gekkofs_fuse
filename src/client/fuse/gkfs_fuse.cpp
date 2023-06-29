#define FUSE_USE_VERSION 26
extern "C" {
#include <stddef.h>
#include <unistd.h>

#include <fuse.h>
}

#include "client/logging.hpp"
#include "client/preload.hpp"
#include "client/open_dir.hpp"
#include "client/gkfs_functions.hpp"

static void*
gkfs_fuse_init(struct fuse_conn_info* conn) {
    LOG_DEBUG("{}() called", __func__);
    init_preload();
    return NULL;
}

void
gkfs_fuse_destroy(void* arg) {
    LOG_DEBUG("{}() called", __func__);
    destroy_preload();
}

static int
gkfs_fuse_getattr(const char* path, struct stat* st) {
    LOG_DEBUG("{}() called with path: {}", __func__, path);

    int ret = gkfs::syscall::gkfs_stat(path, st);
    LOG_DEBUG("gkfs_stat() called with return value: {}", ret);
    if(ret == -1) {
        return -errno;
    }
    return 0;
}

static int
gkfs_fuse_opendir(const char* path, struct fuse_file_info* fi) {
    LOG_DEBUG("{}() called with path: {}", __func__, path);

    int ret = gkfs::syscall::gkfs_opendir(path);
    LOG_DEBUG("gkfs_opendir() called with return value: {}", ret);
    if(ret == -1) {
        return -errno;
    }

    fi->fh = ret;
    return 0;
}

static int
gkfs_fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info* fi) {
    LOG_DEBUG("{}() called with path: {}", __func__, path);

    int fd = fi->fh;
    auto dirp = CTX->file_map()->get_dir(fd);
    for(size_t i = 0; i < dirp->size(); i++) {
        struct stat st;
        int ret = gkfs_fuse_getattr(path, &st);
        if(ret < 0) {
            return ret;
        }

        auto entry = dirp->getdent(i);
        filler(buf, entry.name().c_str(), &st, 0);
    }

    return 0;
}

static int
gkfs_fuse_mkdir(const char* path, mode_t mode) {
    LOG_DEBUG("{}() called with path: {}", __func__, path);

    int ret = gkfs::syscall::gkfs_create(path, mode | S_IFDIR);
    LOG_DEBUG("gkfs_create() called with return value: {}", ret);
    if(ret == -1) {
        return -errno;
    }
    return ret;
}

static int
gkfs_fuse_rmdir(const char* path) {
    LOG_DEBUG("{}() called with path: {}", __func__, path);

    int ret = gkfs::syscall::gkfs_rmdir(path);
    LOG_DEBUG("gkfs_rmdir() called with return value: {}", ret);
    if(ret == -1) {
        return -errno;
    }
    return ret;
}

static int
gkfs_fuse_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    LOG_DEBUG("{}() called with path: {}", __func__, path);

    int ret = gkfs::syscall::gkfs_open(path, mode | S_IFREG,
                                       O_WRONLY | O_CREAT | O_TRUNC);
    LOG_DEBUG("gkfs_open() called with return value: {}", ret);
    if(ret == -1) {
        return -errno;
    }

    fi->fh = ret;
    return 0;
}

static int
gkfs_fuse_open(const char* path, struct fuse_file_info* fi) {
    LOG_DEBUG("{}() called with path: {}", __func__, path);

    int ret = gkfs::syscall::gkfs_open(path, 0644, fi->flags);
    LOG_DEBUG("gkfs_open() called with return value: {}", ret);
    if(ret == -1) {
        return -errno;
    }

    fi->fh = ret;
    return 0;
}

static int
gkfs_fuse_read(const char* path, char* buf, size_t size, off_t offset,
               struct fuse_file_info* fi) {
    int fd = fi->fh;
    LOG_DEBUG("{}() called with fd: {}", __func__, fd);

    int ret = gkfs::syscall::gkfs_pread_ws(fi->fh, (void*) buf, size, offset);
    LOG_DEBUG("gkfs_pread_ws() called with return value: {}", ret);
    if(ret == -1) {
        return -errno;
    }

    return ret;
}

static int
gkfs_fuse_write(const char* path, const char* buf, size_t size, off_t offset,
                struct fuse_file_info* fi) {

    int fd = fi->fh;
    LOG_DEBUG("{}() called with fd: {}", __func__, fd);

    int ret = gkfs::syscall::gkfs_pwrite_ws(fd, buf, size, offset);
    LOG_DEBUG("gkfs_pwrite_ws() called with return value: {}", ret);
    if(ret == -1) {
        return -errno;
    }

    return ret;
}

static int
gkfs_fuse_release(const char* path, struct fuse_file_info* fi) {
    int fd = fi->fh;
    LOG_DEBUG("{}() called with fd: {}", __func__, fd);

    if(CTX->file_map()->exist(fd)) {
        CTX->file_map()->remove(fd);
        return 0;
    }

    if(CTX->is_internal_fd(fd)) {
        return 0;
    }

    return close(fd);
}

static int
gkfs_fuse_unlink(const char* path) {
    LOG_DEBUG("{}() called with path: {}", __func__, path);

    int ret = gkfs::syscall::gkfs_remove(path);
    LOG_DEBUG("gkfs_remove() called with return value: {}", ret);
    if(ret == -1) {
        return -errno;
    }
    return ret;
}

static int
gkfs_fuse_truncate(const char* path, off_t size) {
    LOG_DEBUG("{}() called with path: {}", __func__, path);

    int ret = gkfs::syscall::gkfs_truncate(path, size);
    LOG_DEBUG("gkfs_truncate() called with return value: {}", ret);
    if(ret == -1) {
        return -errno;
    }
    return ret;
}

static int
gkfs_fuse_utimens(const char* path, const struct timespec tv[2]) {
    LOG_DEBUG("{}() called with path: {}", __func__, path);
    return 0;
}

static const struct fuse_operations gkfs_ops = {
        .getattr = gkfs_fuse_getattr,
        .mkdir = gkfs_fuse_mkdir,
        .unlink = gkfs_fuse_unlink,
        .rmdir = gkfs_fuse_rmdir,
        .truncate = gkfs_fuse_truncate,
        .open = gkfs_fuse_open,
        .read = gkfs_fuse_read,
        .write = gkfs_fuse_write,
        .release = gkfs_fuse_release,
        .opendir = gkfs_fuse_opendir,
        .readdir = gkfs_fuse_readdir,
        .releasedir = gkfs_fuse_release,
        .init = gkfs_fuse_init,
        .destroy = gkfs_fuse_destroy,
        .create = gkfs_fuse_create,
        .utimens = gkfs_fuse_utimens,
};

int
main(int argc, char* argv[]) {
    fuse_main(argc, argv, &gkfs_ops, NULL);
    return 0;
}
