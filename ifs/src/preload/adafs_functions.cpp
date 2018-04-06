#include <preload/adafs_functions.hpp>
#include <preload/rpc/ld_rpc_metadentry.hpp>
#include <preload/rpc/ld_rpc_data_ws.hpp>

using namespace std;

int adafs_open(const std::string& path, mode_t mode, int flags) {
    init_ld_env_if_needed();
    int err = 0;

    if (flags & O_CREAT){
        // no access check required here. If one is using our FS they have the permissions.
        err = adafs_mk_node(path, mode | S_IFREG);
        if(err != 0)
            return -1;

    } else {
        auto mask = F_OK; // F_OK == 0
#if defined(CHECK_ACCESS_DURING_OPEN)
        if ((mode & S_IRUSR) || (mode & S_IRGRP) || (mode & S_IROTH))
            mask = mask & R_OK;
        if ((mode & S_IWUSR) || (mode & S_IWGRP) || (mode & S_IWOTH))
            mask = mask & W_OK;
        if ((mode & S_IXUSR) || (mode & S_IXGRP) || (mode & S_IXOTH))
            mask = mask & X_OK;
#endif
#if defined(DO_LOOKUP)
        // check if file exists
        err = rpc_send_access(path, mask);
        if(err != 0)
            return -1;
#endif
    }
    
    // TODO the open flags should not be in the map just set the pos accordingly
    return file_map.add(path, flags);
}

int adafs_mk_node(const std::string& path, const mode_t mode) {
    init_ld_env_if_needed();

    //file type must be set
    assert((mode & S_IFMT) != 0);
    //file type must be either regular file or directory
    assert(S_ISREG(mode) || S_ISDIR(mode));

    return rpc_send_mk_node(path, mode);
}

/**
 * This sends internally a broadcast (i.e. n RPCs) to clean their chunk folders for that path
 * @param path
 * @return
 */
int adafs_rm_node(const std::string& path) {
    init_ld_env_if_needed();
    return rpc_send_rm_node(path);
}

int adafs_access(const std::string& path, const int mask) {
    init_ld_env_if_needed();
#if !defined(DO_LOOKUP)
    // object is assumed to be existing, even though it might not
    return 0;
#endif
#if defined(CHECK_ACCESS)
    return rpc_send_access(path, mask);
#else
    return rpc_send_access(path, F_OK); // Only check for file exists
#endif
}

// TODO combine adafs_stat and adafs_stat64
int adafs_stat(const string& path, struct stat* buf) {
    init_ld_env_if_needed();
    string attr = ""s;
    auto err = rpc_send_stat(path, attr);
    if (err == 0)
        db_val_to_stat(path, attr, *buf);
    return err;
}

int adafs_stat64(const string& path, struct stat64* buf) {
    init_ld_env_if_needed();
    string attr = ""s;
    auto err = rpc_send_stat(path, attr);
    if (err == 0)
        db_val_to_stat64(path, attr, *buf);
    return err;
}

int adafs_statfs(const string& path, struct statfs* adafs_buf, struct statfs& realfs_buf) {
    init_ld_env_if_needed();
    // Check that file path exists
    auto ret = rpc_send_access(path, F_OK);
    // Valid fs error
    if (ret > 0) {
        errno = ret;
        return -1;
    }
    // RPC error (errno has been set)
    if (ret < 0)
        return -1;

    // fs object exists. Let's make up some fs values
    adafs_buf->f_type = 0; // fs is not know to VFS. Therefore, no valid specifier possible
    adafs_buf->f_bsize = static_cast<int>(CHUNKSIZE);
    // some rough estimations
    adafs_buf->f_blocks = realfs_buf.f_blocks * fs_config->host_size;
    adafs_buf->f_bfree = realfs_buf.f_bfree * fs_config->host_size;
    adafs_buf->f_bavail = realfs_buf.f_bavail * fs_config->host_size;
    adafs_buf->f_files = realfs_buf.f_files * fs_config->host_size;
    adafs_buf->f_ffree = realfs_buf.f_ffree * fs_config->host_size;
    adafs_buf->f_fsid = realfs_buf.f_fsid; // "Nobody knows what f_fsid is supposed to contain"
    adafs_buf->f_namelen = realfs_buf.f_namelen;
    adafs_buf->f_frsize = realfs_buf.f_frsize;
    adafs_buf->f_spare[0] = 0;
    adafs_buf->f_spare[1] = 0;
    adafs_buf->f_spare[2] = 0;
    adafs_buf->f_spare[3] = 0;
    adafs_buf->f_flags = ST_NOATIME | ST_NOSUID | ST_NODEV | ST_SYNCHRONOUS;
    if (!fs_config->atime_state)
        adafs_buf->f_flags = adafs_buf->f_flags | ST_NOATIME | ST_NODIRATIME;
    return 0;
}

off64_t adafs_lseek(int fd, off64_t offset, int whence) {
    init_ld_env_if_needed();
    return adafs_lseek(file_map.get(fd), offset, whence);
}

off64_t adafs_lseek(shared_ptr<OpenFile> adafs_fd, off64_t offset, int whence) {
    init_ld_env_if_needed();
    switch (whence) {
        case SEEK_SET:
            adafs_fd->pos(offset);
            break;
        case SEEK_CUR:
            adafs_fd->pos(adafs_fd->pos() + offset);
            break;
        case SEEK_END: {
            off64_t file_size;
            auto err = rpc_send_get_metadentry_size(adafs_fd->path(), file_size);
            if (err < 0) {
                errno = err; // Negative numbers are explicitly for error codes
                return -1;
            }
            adafs_fd->pos(file_size + offset);
            break;
        }
        case SEEK_DATA:
            // We do not support this whence yet
            errno = EINVAL;
            return -1;
        case SEEK_HOLE:
            // We do not support this whence yet
            errno = EINVAL;
            return -1;
        default:
            errno = EINVAL;
            return -1;
    }
    return adafs_fd->pos();
}

int adafs_dup(const int oldfd) {
    return file_map.dup(oldfd);
}

int adafs_dup2(const int oldfd, const int newfd) {
    return file_map.dup2(oldfd, newfd);
}


ssize_t adafs_pwrite_ws(int fd, const void* buf, size_t count, off64_t offset) {
    init_ld_env_if_needed();
    auto adafs_fd = file_map.get(fd);
    auto path = make_shared<string>(adafs_fd->path());
    auto append_flag = adafs_fd->get_flag(OpenFile_flags::append);
    ssize_t ret = 0;
    long updated_size = 0;

    ret = rpc_send_update_metadentry_size(*path, count, offset, append_flag, updated_size);
    if (ret != 0) {
        ld_logger->error("{}() update_metadentry_size failed with ret {}", __func__, ret);
        return 0; // ERR
    }
    ret = rpc_send_write(*path, buf, append_flag, offset, count, updated_size);
    if (ret < 0) {
        ld_logger->warn("{}() rpc_send_write failed with ret {}", __func__, ret);
    }
    return ret; // return written size or -1 as error
}

ssize_t adafs_pread_ws(int fd, void* buf, size_t count, off64_t offset) {
    init_ld_env_if_needed();
    auto adafs_fd = file_map.get(fd);
    auto path = make_shared<string>(adafs_fd->path());

    auto ret = rpc_send_read(*path, buf, offset, count);
    if (ret < 0) {
        ld_logger->warn("{}() rpc_send_read failed with ret {}", __func__, ret);
    }
    // XXX check that we don't try to read past end of the file
    return ret; // return read size or -1 as error
}