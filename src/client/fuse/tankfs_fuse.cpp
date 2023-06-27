#define FUSE_USE_VERSION 26
extern "C" {
#include <stdio.h>
#include <stddef.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <fuse.h>
}

#include "client/logging.hpp"
#include "client/preload.hpp"
#include "client/hooks.hpp"
#include "client/open_dir.hpp"
#include "client/gkfs_functions.hpp"

static void * tankfs_fuse_init(struct fuse_conn_info *conn) {
	LOG_INFO("{}() called", __func__);
    init_preload();
    return NULL;
}

void tankfs_fuse_destroy(void *arg) {
	LOG_INFO("{}() called", __func__);
	destroy_preload();
}

static int tankfs_fuse_getattr(const char* path, struct stat* st) {
	LOG_INFO("{}() called with path: {}", __func__, path);

	int ret = gkfs::syscall::gkfs_stat(path, st);
	if (ret == -1) {
		return -errno;
	}
	return 0;
}

static int tankfs_fuse_opendir(const char *path, struct fuse_file_info *fi) {
	LOG_INFO("{}() called with path: {}", __func__, path);

	int ret = gkfs::syscall::gkfs_opendir(path);
	if (ret == -1) {
		return -errno;
	}
	
	fi->fh = ret;
    return 0;
}

static int tankfs_fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
	off_t offset, struct fuse_file_info *fi) {
	LOG_INFO("{}() called with path: {}", __func__, path);

	int fd = fi->fh;
	auto dirp = CTX->file_map()->get_dir(fd);
	for (int i = 0; i < dirp->size(); i++) {
		struct stat st;
		int ret = tankfs_fuse_getattr(path, &st);
		if (ret < 0) {
			return ret;
		}

		auto entry = dirp->getdent(i);
		filler(buf, entry.name().c_str(), &st, 0);
	}

	return 0;
}

static int tankfs_fuse_mkdir(const char *path, mode_t mode) {
	LOG_INFO("{}() called with path: {}", __func__, path);

	int ret = gkfs::syscall::gkfs_create(path, mode | S_IFDIR);
	if (ret == -1) {
		return -errno;
	}
	return ret;
}

static int tankfs_fuse_rmdir(const char *path) {
	LOG_INFO("{}() called with path: {}", __func__, path);

	int ret = gkfs::syscall::gkfs_rmdir(path);
	if (ret == -1) {
		return -errno;
	}
	return ret;
}

static int tankfs_fuse_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	LOG_INFO("{}() called with path: {}", __func__, path);

	int ret = gkfs::syscall::gkfs_open(path, mode | S_IFREG, O_WRONLY | O_CREAT | O_TRUNC);
	LOG_INFO("create: {}", ret);
	if (ret == -1) {
		return -errno;
	}

	fi->fh = ret;
	return 0;
}

static int tankfs_fuse_open(const char *path, struct fuse_file_info *fi) {
	LOG_INFO("{}() called with path: {}", __func__, path);

	int ret = gkfs::syscall::gkfs_open(path, 0644, fi->flags);
	LOG_INFO("open: {}", ret);
	if (ret == -1) {
		return -errno;
	}

	fi->fh = ret;
	return 0;
}

static int tankfs_fuse_read(const char *path, char *buf, size_t size, off_t offset,
	struct fuse_file_info *fi) {
	int fd = fi->fh;
	LOG_INFO("{}() called with fd: {}", __func__, fd);

	int ret = gkfs::syscall::gkfs_pread_ws(fi->fh, (void *)buf, size, offset);
	LOG_INFO("read: {}", ret);
	if (ret == -1) {
		return -errno;
	}

	return ret;
}

static int tankfs_fuse_write(const char *path, const char *buf, size_t size,
       off_t offset, struct fuse_file_info *fi) {

	int fd = fi->fh;
	LOG_INFO("{}() called with fd: {}", __func__, fd);
	
	int ret = gkfs::syscall::gkfs_pwrite_ws(fd, buf, size, offset);
	if (ret == -1) {
		return -errno;
	}

	return ret;
}

static int tankfs_fuse_release(const char *path, struct fuse_file_info *fi) {
	int fd = fi->fh;
	LOG_INFO("{}() called with fd: {}", __func__, fd);

    if(CTX->file_map()->exist(fd)) {
        CTX->file_map()->remove(fd);
        return 0;
    }

    if(CTX->is_internal_fd(fd)) {
        return 0;
    }

	return close(fd);
}

static int tankfs_fuse_unlink(const char *path) {
	LOG_INFO("{}() called with path: {}", __func__, path);

	int ret = gkfs::syscall::gkfs_remove(path);
	if (ret == -1) {
		return -errno;
	}
	return ret;
}

static int tankfs_fuse_truncate(const char *path, off_t size) {
	LOG_INFO("{}() called with path: {}", __func__, path);

	int ret = gkfs::syscall::gkfs_truncate(path, size);
	if (ret == -1) {
		return -errno;
	}
	return ret;
}

static int tankfs_fuse_utimens(const char *path, const struct timespec tv[2]) {
	LOG_INFO("{}() called with path: {}", __func__, path);
	return 0;
}

static const struct fuse_operations tankfs_ops = {
    .getattr    = tankfs_fuse_getattr,		
	.mkdir      = tankfs_fuse_mkdir,		
	.unlink     = tankfs_fuse_unlink,		
	.rmdir      = tankfs_fuse_rmdir,		
	.truncate   = tankfs_fuse_truncate,     
	.open       = tankfs_fuse_open,			
	.read       = tankfs_fuse_read,         
	.write      = tankfs_fuse_write,        
	.release    = tankfs_fuse_release,      
	.opendir    = tankfs_fuse_opendir,      
    .readdir    = tankfs_fuse_readdir,      
    .releasedir = tankfs_fuse_release,      
    .init       = tankfs_fuse_init,
    .destroy    = tankfs_fuse_destroy,
	.create     = tankfs_fuse_create,       
	.utimens    = tankfs_fuse_utimens,      
};

int main(int argc, char *argv[]) {
    int ret;

    ret = fuse_main(argc, argv, &tankfs_ops, NULL);
    return 0;
}
