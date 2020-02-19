/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/


#include <config.hpp>
#include <client/preload.hpp>
#include <client/preload_util.hpp>
#include <client/logging.hpp>
#include <client/gkfs_functions.hpp>
#include <client/rpc/ld_rpc_metadentry.hpp>
#include <client/rpc/ld_rpc_data_ws.hpp>
#include <client/open_dir.hpp>

#include <global/path_util.hpp>

extern "C" {
#include <sys/statfs.h>
#include <sys/statvfs.h>
}

#define __ALIGN_KERNEL_MASK(x, mask)    (((x) + (mask)) & ~(mask))
#define __ALIGN_KERNEL(x, a)            __ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define ALIGN(x, a)                     __ALIGN_KERNEL((x), (a))

using namespace std;

std::shared_ptr<Metadata> gkfs::func::metadata(const string& path, bool follow_links) {
    std::string attr;
    auto err = gkfs::rpc::forward_stat(path, attr);
    if (err) {
        return nullptr;
    }
#ifdef HAS_SYMLINKS
    if (follow_links) {
        Metadata md{attr};
        while (md.is_link()) {
            err = gkfs::rpc::forward_stat(md.target_path(), attr);
            if (err) {
                return nullptr;
            }
            md = Metadata{attr};
        }
    }
#endif
    return make_shared<Metadata>(attr);
}

int gkfs::func::check_parent_dir(const std::string& path) {
#if CREATE_CHECK_PARENTS
    auto p_comp = path::dirname(path);
    auto md = gkfs::func::metadata(p_comp);
    if (!md) {
        if (errno == ENOENT) {
            LOG(DEBUG, "Parent component does not exist: '{}'", p_comp);
        } else {
            LOG(ERROR, "Failed to get metadata for parent component '{}': {}", path, strerror(errno));
        }
        return -1;
    }
    if (!S_ISDIR(md->mode())) {
        LOG(DEBUG, "Parent component is not a directory: '{}'", p_comp);
        errno = ENOTDIR;
        return -1;
    }
#endif // CREATE_CHECK_PARENTS
    return 0;
}

int gkfs::func::open(const std::string& path, mode_t mode, int flags) {

    if (flags & O_PATH) {
        LOG(ERROR, "`O_PATH` flag is not supported");
        errno = ENOTSUP;
        return -1;
    }

    if (flags & O_APPEND) {
        LOG(ERROR, "`O_APPEND` flag is not supported");
        errno = ENOTSUP;
        return -1;
    }

    bool exists = true;
    auto md = gkfs::func::metadata(path);
    if (!md) {
        if (errno == ENOENT) {
            exists = false;
        } else {
            LOG(ERROR, "Error while retriving stat to file");
            return -1;
        }
    }

    if (!exists) {
        if (!(flags & O_CREAT)) {
            // file doesn't exists and O_CREAT was not set
            errno = ENOENT;
            return -1;
        }

        /***   CREATION    ***/
        assert(flags & O_CREAT);

        if (flags & O_DIRECTORY) {
            LOG(ERROR, "O_DIRECTORY use with O_CREAT. NOT SUPPORTED");
            errno = ENOTSUP;
            return -1;
        }

        // no access check required here. If one is using our FS they have the permissions.
        if (gkfs::func::mk_node(path, mode | S_IFREG)) {
            LOG(ERROR, "Error creating non-existent file: '{}'", strerror(errno));
            return -1;
        }
    } else {
        /* File already exists */

        if (flags & O_EXCL) {
            // File exists and O_EXCL was set
            errno = EEXIST;
            return -1;
        }

#ifdef HAS_SYMLINKS
        if (md->is_link()) {
            if (flags & O_NOFOLLOW) {
                LOG(WARNING, "Symlink found and O_NOFOLLOW flag was specified");
                errno = ELOOP;
                return -1;
            }
            return gkfs::func::open(md->target_path(), mode, flags);
        }
#endif

        if (S_ISDIR(md->mode())) {
            return gkfs::func::opendir(path);
        }


        /*** Regular file exists ***/
        assert(S_ISREG(md->mode()));

        if ((flags & O_TRUNC) && ((flags & O_RDWR) || (flags & O_WRONLY))) {
            if (gkfs::func::truncate(path, md->size(), 0)) {
                LOG(ERROR, "Error truncating file");
                return -1;
            }
        }
    }

    return CTX->file_map()->add(std::make_shared<OpenFile>(path, flags));
}

int gkfs::func::mk_node(const std::string& path, mode_t mode) {

    //file type must be set
    switch (mode & S_IFMT) {
        case 0:
            mode |= S_IFREG;
            break;
        case S_IFREG: // intentionally fall-through
        case S_IFDIR:
            break;
        case S_IFCHR: // intentionally fall-through
        case S_IFBLK:
        case S_IFIFO:
        case S_IFSOCK:
            LOG(WARNING, "Unsupported node type");
            errno = ENOTSUP;
            return -1;
        default:
            LOG(WARNING, "Unrecognized node type");
            errno = EINVAL;
            return -1;
    }

    if (check_parent_dir(path)) {
        return -1;
    }
    return gkfs::rpc::forward_create(path, mode);
}

/**
 * This sends internally a broadcast (i.e. n RPCs) to clean their chunk folders for that path
 * @param path
 * @return
 */
int gkfs::func::rm_node(const std::string& path) {
    auto md = gkfs::func::metadata(path);
    if (!md) {
        return -1;
    }
    bool has_data = S_ISREG(md->mode()) && (md->size() != 0);
    return gkfs::rpc::forward_remove(path, !has_data, md->size());
}

int gkfs::func::access(const std::string& path, const int mask, bool follow_links) {
    auto md = gkfs::func::metadata(path, follow_links);
    if (!md) {
        errno = ENOENT;
        return -1;
    }
    return 0;
}

int gkfs::func::stat(const string& path, struct stat* buf, bool follow_links) {
    auto md = gkfs::func::metadata(path, follow_links);
    if (!md) {
        return -1;
    }
    gkfs::client::metadata_to_stat(path, *md, *buf);
    return 0;
}

int gkfs::func::statfs(sys_statfs* buf) {
    auto blk_stat = gkfs::rpc::forward_get_chunk_stat();
    buf->f_type = 0;
    buf->f_bsize = blk_stat.chunk_size;
    buf->f_blocks = blk_stat.chunk_total;
    buf->f_bfree = blk_stat.chunk_free;
    buf->f_bavail = blk_stat.chunk_free;
    buf->f_files = 0;
    buf->f_ffree = 0;
    buf->f_fsid = {0, 0};
    buf->f_namelen = path::max_length;
    buf->f_frsize = 0;
    buf->f_flags =
            ST_NOATIME | ST_NODIRATIME | ST_NOSUID | ST_NODEV | ST_SYNCHRONOUS;
    return 0;
}

int gkfs::func::statvfs(sys_statvfs* buf) {
    init_ld_env_if_needed();
    auto blk_stat = gkfs::rpc::forward_get_chunk_stat();
    buf->f_bsize = blk_stat.chunk_size;
    buf->f_blocks = blk_stat.chunk_total;
    buf->f_bfree = blk_stat.chunk_free;
    buf->f_bavail = blk_stat.chunk_free;
    buf->f_files = 0;
    buf->f_ffree = 0;
    buf->f_favail = 0;
    buf->f_fsid = 0;
    buf->f_namemax = path::max_length;
    buf->f_frsize = 0;
    buf->f_flag =
            ST_NOATIME | ST_NODIRATIME | ST_NOSUID | ST_NODEV | ST_SYNCHRONOUS;
    return 0;
}

off_t gkfs::func::lseek(unsigned int fd, off_t offset, unsigned int whence) {
    return gkfs::func::lseek(CTX->file_map()->get(fd), offset, whence);
}

off_t gkfs::func::lseek(shared_ptr<OpenFile> gkfs_fd, off_t offset, unsigned int whence) {
    switch (whence) {
        case SEEK_SET:
            gkfs_fd->pos(offset);
            break;
        case SEEK_CUR:
            gkfs_fd->pos(gkfs_fd->pos() + offset);
            break;
        case SEEK_END: {
            off64_t file_size;
            auto err = gkfs::rpc::forward_get_metadentry_size(gkfs_fd->path(), file_size);
            if (err < 0) {
                errno = err; // Negative numbers are explicitly for error codes
                return -1;
            }
            gkfs_fd->pos(file_size + offset);
            break;
        }
        case SEEK_DATA:
            LOG(WARNING, "SEEK_DATA whence is not supported");
            // We do not support this whence yet
            errno = EINVAL;
            return -1;
        case SEEK_HOLE:
            LOG(WARNING, "SEEK_HOLE whence is not supported");
            // We do not support this whence yet
            errno = EINVAL;
            return -1;
        default:
            LOG(WARNING, "Unknown whence value {:#x}", whence);
            errno = EINVAL;
            return -1;
    }
    return gkfs_fd->pos();
}

int gkfs::func::truncate(const std::string& path, off_t old_size, off_t new_size) {
    assert(new_size >= 0);
    assert(new_size <= old_size);

    if (new_size == old_size) {
        return 0;
    }

    if (gkfs::rpc::forward_decr_size(path, new_size)) {
        LOG(DEBUG, "Failed to decrease size");
        return -1;
    }

    if (gkfs::rpc::forward_truncate(path, old_size, new_size)) {
        LOG(DEBUG, "Failed to truncate data");
        return -1;
    }
    return 0;
}

int gkfs::func::truncate(const std::string& path, off_t length) {
    /* TODO CONCURRENCY:
     * At the moment we first ask the length to the metadata-server in order to
     * know which data-server have data to be deleted.
     *
     * From the moment we issue the gkfs_stat and the moment we issue the
     * gkfs_trunc_data, some more data could have been added to the file and the
     * length increased.
     */
    if (length < 0) {
        LOG(DEBUG, "Length is negative: {}", length);
        errno = EINVAL;
        return -1;
    }

    auto md = gkfs::func::metadata(path, true);
    if (!md) {
        return -1;
    }
    auto size = md->size();
    if (static_cast<unsigned long>(length) > size) {
        LOG(DEBUG, "Length is greater then file size: {} > {}", length, size);
        errno = EINVAL;
        return -1;
    }
    return gkfs::func::truncate(path, size, length);
}

int gkfs::func::dup(const int oldfd) {
    return CTX->file_map()->dup(oldfd);
}

int gkfs::func::dup2(const int oldfd, const int newfd) {
    return CTX->file_map()->dup2(oldfd, newfd);
}

ssize_t gkfs::func::pwrite(std::shared_ptr<OpenFile> file, const char* buf, size_t count, off64_t offset) {
    if (file->type() != FileType::regular) {
        assert(file->type() == FileType::directory);
        LOG(WARNING, "Cannot read from directory");
        errno = EISDIR;
        return -1;
    }
    auto path = make_shared<string>(file->path());
    auto append_flag = file->get_flag(OpenFile_flags::append);
    ssize_t ret = 0;
    long updated_size = 0;

    ret = gkfs::rpc::forward_update_metadentry_size(*path, count, offset, append_flag, updated_size);
    if (ret != 0) {
        LOG(ERROR, "update_metadentry_size() failed with ret {}", ret);
        return ret; // ERR
    }
    ret = gkfs::rpc::forward_write(*path, buf, append_flag, offset, count, updated_size);
    if (ret < 0) {
        LOG(WARNING, "gkfs::rpc::forward_write() failed with ret {}", ret);
    }
    return ret; // return written size or -1 as error
}

ssize_t gkfs::func::pwrite_ws(int fd, const void* buf, size_t count, off64_t offset) {
    auto file = CTX->file_map()->get(fd);
    return gkfs::func::pwrite(file, reinterpret_cast<const char*>(buf), count, offset);
}

/* Write counts bytes starting from current file position
 * It also update the file position accordingly
 *
 * Same as write syscall.
*/
ssize_t gkfs::func::write(int fd, const void* buf, size_t count) {
    auto gkfs_fd = CTX->file_map()->get(fd);
    auto pos = gkfs_fd->pos(); //retrieve the current offset
    if (gkfs_fd->get_flag(OpenFile_flags::append))
        gkfs::func::lseek(gkfs_fd, 0, SEEK_END);
    auto ret = gkfs::func::pwrite(gkfs_fd, reinterpret_cast<const char*>(buf), count, pos);
    // Update offset in file descriptor in the file map
    if (ret > 0) {
        gkfs_fd->pos(pos + count);
    }
    return ret;
}

ssize_t gkfs::func::pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset) {

    auto file = CTX->file_map()->get(fd);
    auto pos = offset; // keep truck of current position
    ssize_t written = 0;
    ssize_t ret;
    for (int i = 0; i < iovcnt; ++i) {
        auto count = (iov + i)->iov_len;
        if (count == 0) {
            continue;
        }
        auto buf = (iov + i)->iov_base;
        ret = gkfs::func::pwrite(file, reinterpret_cast<char*>(buf), count, pos);
        if (ret == -1) {
            break;
        }
        written += ret;
        pos += ret;

        if (static_cast<size_t>(ret) < count) {
            break;
        }
    }

    if (written == 0) {
        return -1;
    }
    return written;
}

ssize_t gkfs::func::writev(int fd, const struct iovec* iov, int iovcnt) {

    auto gkfs_fd = CTX->file_map()->get(fd);
    auto pos = gkfs_fd->pos(); // retrieve the current offset
    auto ret = gkfs::func::pwritev(fd, iov, iovcnt, pos);
    assert(ret != 0);
    if (ret < 0) {
        return -1;
    }
    gkfs_fd->pos(pos + ret);
    return ret;
}

ssize_t gkfs::func::pread(std::shared_ptr<OpenFile> file, char* buf, size_t count, off64_t offset) {
    if (file->type() != FileType::regular) {
        assert(file->type() == FileType::directory);
        LOG(WARNING, "Cannot read from directory");
        errno = EISDIR;
        return -1;
    }

    // Zeroing buffer before read is only relevant for sparse files. Otherwise sparse regions contain invalid data.
    if (gkfs::config::io::zero_buffer_before_read) {
        memset(buf, 0, sizeof(char) * count);
    }
    auto ret = gkfs::rpc::forward_read(file->path(), buf, offset, count);
    if (ret < 0) {
        LOG(WARNING, "gkfs::rpc::forward_read() failed with ret {}", ret);
    }
    // XXX check that we don't try to read past end of the file
    return ret; // return read size or -1 as error
}

ssize_t gkfs::func::read(int fd, void* buf, size_t count) {
    auto gkfs_fd = CTX->file_map()->get(fd);
    auto pos = gkfs_fd->pos(); //retrieve the current offset
    auto ret = gkfs::func::pread(gkfs_fd, reinterpret_cast<char*>(buf), count, pos);
    // Update offset in file descriptor in the file map
    if (ret > 0) {
        gkfs_fd->pos(pos + ret);
    }
    return ret;
}

ssize_t gkfs::func::pread_ws(int fd, void* buf, size_t count, off64_t offset) {
    auto gkfs_fd = CTX->file_map()->get(fd);
    return gkfs::func::pread(gkfs_fd, reinterpret_cast<char*>(buf), count, offset);
}

int gkfs::func::opendir(const std::string& path) {

    auto md = gkfs::func::metadata(path);
    if (!md) {
        return -1;
    }
    if (!S_ISDIR(md->mode())) {
        LOG(DEBUG, "Path is not a directory");
        errno = ENOTDIR;
        return -1;
    }

    auto open_dir = std::make_shared<OpenDir>(path);
    gkfs::rpc::forward_get_dirents(*open_dir);
    return CTX->file_map()->add(open_dir);
}

int gkfs::func::rmdir(const std::string& path) {
    auto md = gkfs::func::metadata(path);
    if (!md) {
        LOG(DEBUG, "Path '{}' does not exist: ", path);
        errno = ENOENT;
        return -1;
    }
    if (!S_ISDIR(md->mode())) {
        LOG(DEBUG, "Path '{}' is not a directory", path);
        errno = ENOTDIR;
        return -1;
    }

    auto open_dir = std::make_shared<OpenDir>(path);
    gkfs::rpc::forward_get_dirents(*open_dir);
    if (open_dir->size() != 0) {
        errno = ENOTEMPTY;
        return -1;
    }
    return gkfs::rpc::forward_remove(path, true, 0);
}

int gkfs::func::getdents(unsigned int fd,
                         struct linux_dirent* dirp,
                         unsigned int count) {

    auto open_dir = CTX->file_map()->get_dir(fd);
    if (open_dir == nullptr) {
        //Cast did not succeeded: open_file is a regular file
        errno = EBADF;
        return -1;
    }

    auto pos = open_dir->pos();
    if (pos >= open_dir->size()) {
        return 0;
    }

    unsigned int written = 0;
    struct linux_dirent* current_dirp = nullptr;
    while (pos < open_dir->size()) {
        DirEntry de = open_dir->getdent(pos);
        auto total_size = ALIGN(offsetof(
                                        struct linux_dirent, d_name) +
                                        de.name().size() + 3, sizeof(long));
        if (total_size > (count - written)) {
            //no enough space left on user buffer to insert next dirent
            break;
        }
        current_dirp = reinterpret_cast<struct linux_dirent*>(
                reinterpret_cast<char*>(dirp) + written);
        current_dirp->d_ino = std::hash<std::string>()(
                open_dir->path() + "/" + de.name());

        current_dirp->d_reclen = total_size;

        *(reinterpret_cast<char*>(current_dirp) + total_size - 1) =
                ((de.type() == FileType::regular) ? DT_REG : DT_DIR);

        LOG(DEBUG, "name {}: {}", pos, de.name());
        std::strcpy(&(current_dirp->d_name[0]), de.name().c_str());
        ++pos;
        current_dirp->d_off = pos;
        written += total_size;
    }

    if (written == 0) {
        errno = EINVAL;
        return -1;
    }
    open_dir->pos(pos);
    return written;
}


int gkfs::func::getdents64(unsigned int fd,
                           struct linux_dirent64* dirp,
                           unsigned int count) {

    auto open_dir = CTX->file_map()->get_dir(fd);
    if (open_dir == nullptr) {
        //Cast did not succeeded: open_file is a regular file
        errno = EBADF;
        return -1;
    }

    auto pos = open_dir->pos();
    if (pos >= open_dir->size()) {
        return 0;
    }

    unsigned int written = 0;
    struct linux_dirent64* current_dirp = nullptr;
    while (pos < open_dir->size()) {
        DirEntry de = open_dir->getdent(pos);
        auto total_size = ALIGN(offsetof(
                                        struct linux_dirent64, d_name) +
                                        de.name().size() + 3, sizeof(long));
        if (total_size > (count - written)) {
            //no enough space left on user buffer to insert next dirent
            break;
        }
        current_dirp = reinterpret_cast<struct linux_dirent64*>(
                reinterpret_cast<char*>(dirp) + written);
        current_dirp->d_ino = std::hash<std::string>()(
                open_dir->path() + "/" + de.name());

        current_dirp->d_reclen = total_size;
        current_dirp->d_type = ((de.type() == FileType::regular) ? DT_REG : DT_DIR);

        LOG(DEBUG, "name {}: {}", pos, de.name());
        std::strcpy(&(current_dirp->d_name[0]), de.name().c_str());
        ++pos;
        current_dirp->d_off = pos;
        written += total_size;
    }

    if (written == 0) {
        errno = EINVAL;
        return -1;
    }
    open_dir->pos(pos);
    return written;
}


#ifdef HAS_SYMLINKS

int gkfs::func::mk_symlink(const std::string& path, const std::string& target_path) {
    init_ld_env_if_needed();
    /* The following check is not POSIX compliant.
     * In POSIX the target is not checked at all.
    *  Here if the target is a directory we raise a NOTSUP error.
    *  So that application know we don't support link to directory.
    */
    auto target_md = gkfs::func::metadata(target_path, false);
    if (target_md != nullptr) {
        auto trg_mode = target_md->mode();
        if (!(S_ISREG(trg_mode) || S_ISLNK(trg_mode))) {
            assert(S_ISDIR(trg_mode));
            LOG(DEBUG, "Target path is a directory. Not supported");
            errno = ENOTSUP;
            return -1;
        }
    }

    if (check_parent_dir(path)) {
        return -1;
    }

    auto link_md = gkfs::func::metadata(path, false);
    if (link_md != nullptr) {
        LOG(DEBUG, "Link exists: '{}'", path);
        errno = EEXIST;
        return -1;
    }

    return gkfs::rpc::forward_mk_symlink(path, target_path);
}

int gkfs::func::readlink(const std::string& path, char* buf, int bufsize) {
    init_ld_env_if_needed();
    auto md = gkfs::func::metadata(path, false);
    if (md == nullptr) {
        LOG(DEBUG, "Named link doesn't exist");
        return -1;
    }
    if (!(md->is_link())) {
        LOG(DEBUG, "The named file is not a symbolic link");
        errno = EINVAL;
        return -1;
    }
    int path_size = md->target_path().size() + CTX->mountdir().size();
    if (path_size >= bufsize) {
        LOG(WARNING, "Destination buffer size is too short: {} < {}, {} ", bufsize, path_size, md->target_path());
        errno = ENAMETOOLONG;
        return -1;
    }

    CTX->mountdir().copy(buf, CTX->mountdir().size());
    std::strcpy(buf + CTX->mountdir().size(), md->target_path().c_str());
    return path_size;
}

#endif