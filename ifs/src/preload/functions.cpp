/**
 * All intercepted functions are defined here
 */
#include <preload/preload.hpp>
#include <preload/rpc/ld_rpc_metadentry.hpp>
#include <preload/rpc/ld_rpc_data.hpp>
#include <preload/passthrough.hpp>
#include <preload/open_file_map.hpp>

using namespace std;

static OpenFileMap file_map{};

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
    if (ld_is_env_initialized() && is_fs_path(path)) {
        auto err = 1;
        auto fd = file_map.add(path, (flags & O_APPEND) != 0);
        // TODO look up if file exists configurable
        err = rpc_send_open(path, mode, flags);
        if (err == 0)
            return fd;
        else {
            file_map.remove(fd);
            return -1;
        }
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

int unlink(const char* path) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    if (ld_is_env_initialized() && is_fs_path(path)) {
        return rpc_send_unlink(path);
    }
    return (reinterpret_cast<decltype(&unlink)>(libc_unlink))(path);
}

int close(int fd) {
    init_passthrough_if_needed();
    if (ld_is_env_initialized() && file_map.exist(fd)) {
        // Currently no call to the daemon is required
        file_map.remove(fd);
        return 0;
    }
    return (reinterpret_cast<decltype(&close)>(libc_close))(fd);
}

int __close(int fd) {
    return close(fd);
}

// TODO combine adafs_stat and adafs_stat64
int adafs_stat(const std::string& path, struct stat* buf) {
    string attr = ""s;
    auto err = rpc_send_stat(path, attr);
    db_val_to_stat(path, attr, *buf);
    return err;
}

int adafs_stat64(const std::string& path, struct stat64* buf) {
    string attr = ""s;
    auto err = rpc_send_stat(path, attr);
    db_val_to_stat64(path, attr, *buf);
    return err;
}

int stat(const char* path, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    if (ld_is_env_initialized() && is_fs_path(path)) {
// TODO call daemon and return
        return adafs_stat(path, buf);
    }
    return (reinterpret_cast<decltype(&stat)>(libc_stat))(path, buf);
}

int fstat(int fd, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with fd {}", __func__, fd);
    if (ld_is_env_initialized() && file_map.exist(fd)) {
        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
// TODO call daemon and return
        return adafs_stat(path, buf);
    }
    return (reinterpret_cast<decltype(&fstat)>(libc_fstat))(fd, buf);
}

int __xstat(int ver, const char* path, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    if (ld_is_env_initialized() && is_fs_path(path)) {
// TODO call stat
        return adafs_stat(path, buf);
    }
    return (reinterpret_cast<decltype(&__xstat)>(libc___xstat))(ver, path, buf);
}

int __xstat64(int ver, const char* path, struct stat64* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    if (ld_is_env_initialized() && is_fs_path(path)) {
        return adafs_stat64(path, buf);
//        // Not implemented
//        return -1;
    }
    return (reinterpret_cast<decltype(&__xstat64)>(libc___xstat64))(ver, path, buf);
}

int __fxstat(int ver, int fd, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with fd {}", __func__, fd);
    if (ld_is_env_initialized() && file_map.exist(fd)) {
// TODO call fstat
        auto path = file_map.get(fd)->path();
        return adafs_stat(path, buf);
    }
    return (reinterpret_cast<decltype(&__fxstat)>(libc___fxstat))(ver, fd, buf);
}

int __fxstat64(int ver, int fd, struct stat64* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with fd {}", __func__, fd);
    if (ld_is_env_initialized() && file_map.exist(fd)) {
// TODO call fstat64
        auto path = file_map.get(fd)->path();
        return adafs_stat64(path, buf);
    }
    return (reinterpret_cast<decltype(&__fxstat64)>(libc___fxstat64))(ver, fd, buf);
}

extern int __lxstat(int ver, const char* path, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    if (ld_is_env_initialized() && is_fs_path(path)) {
// Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&__lxstat)>(libc___lxstat))(ver, path, buf);
}

extern int __lxstat64(int ver, const char* path, struct stat64* buf) __THROW {
    init_passthrough_if_needed();
    if (ld_is_env_initialized() && is_fs_path(path)) {
// Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&__lxstat64)>(libc___lxstat64))(ver, path, buf);
}

int access(const char* path, int mode) __THROW {
    init_passthrough_if_needed();
    if (ld_is_env_initialized() && is_fs_path(path)) {
// TODO
    }
    return (reinterpret_cast<decltype(&access)>(libc_access))(path, mode);
}

int puts(const char* str) {
    return (reinterpret_cast<decltype(&puts)>(libc_puts))(str);
}

ssize_t write(int fd, const void* buf, size_t count) {
    init_passthrough_if_needed();
    if (ld_is_env_initialized() && file_map.exist(fd)) {
        ld_logger->trace("{}() called with fd {}", __func__, fd);
        // TODO if append flag has been given, set offset accordingly.
        // XXX handle lseek too
        return pwrite(fd, buf, count, 0);
    }
    return (reinterpret_cast<decltype(&write)>(libc_write))(fd, buf, count);
}

ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset) {
    init_passthrough_if_needed();
    if (ld_is_env_initialized() && file_map.exist(fd)) {
        ld_logger->trace("{}() called with fd {}", __func__, fd);
        auto adafs_fd = file_map.get(fd);
        auto path = adafs_fd->path();
        auto append_flag = adafs_fd->append_flag();
        int err = 0;
        long updated_size = 0;
        auto write_size = static_cast<size_t>(0);

//        if (append_flag)
        err = rpc_send_update_metadentry_size(path, count, append_flag, updated_size);
        if (err != 0) {
            ld_logger->error("{}() update_metadentry_size failed", __func__);
            return 0; // ERR
        }

        // started here // TODO handle offset
        auto chunk_n = static_cast<size_t>(ceil(
                count / static_cast<float>(CHUNKSIZE))); // get number of chunks needed for writing
        ABT_xstream xstream;
        ABT_pool pool;
        auto ret = ABT_xstream_self(&xstream);
        if (ret != 0) {
            ld_logger->error("{}() Unable to get self xstream. Is Argobots initialized?", __func__);
            return -1;
        }
        ret = ABT_xstream_get_main_pools(xstream, 1, &pool);
        if (ret != 0) {
            ld_logger->error("{}() Unable to get main pools from ABT xstream", __func__);
            return -1;
        }
        // Collect all chunk ids within count that have the same destination so that those are send in one rpc bulk transfer
        map<unsigned long, vector<unsigned long>> dest_ids{};
        for (unsigned long i = 0; i < chunk_n; i++) {
            auto recipient = get_rpc_node(path + fmt::FormatInt(i).str());
            if (dest_ids.count(recipient) == 0)
                dest_ids.insert(make_pair(recipient, vector<unsigned long>{i}));
            else
                dest_ids[recipient].push_back(i);
        }
        // Create an Argobots thread per destination, fill an appropriate struct with its destination chunk ids
        auto dest_n = dest_ids.size();
        vector<ABT_thread> threads(dest_n);
        vector<ABT_eventual> eventuals(dest_n);
        vector<struct write_args*> thread_args(dest_n);
        for (unsigned long i = 0; i < dest_n; i++) {
            ABT_eventual_create(sizeof(size_t), &eventuals[i]);
            struct write_args args = {
                    path, // path
                    count, // total size to write
                    offset, // writing offset
                    buf, // pointer to write buffer
                    append_flag, // append flag when file was opened
                    updated_size, // for append truncate TODO needed?
                    dest_ids[i], // pointer to list of chunk ids that all go to the same destination
                    &eventuals[i], // pointer to an eventual which has allocated memory for storing the written size
            };
            thread_args[i] = &args;
            ABT_thread_create(pool, rpc_send_write_abt, &(*thread_args[i]), ABT_THREAD_ATTR_NULL, &threads[i]);
        }

        for (unsigned long i = 0; i < dest_n; i++) {
            size_t* thread_ret_size;
            ABT_eventual_wait(eventuals[i], (void**) &thread_ret_size);
            if (thread_ret_size == nullptr || *thread_ret_size == 0) {
                // TODO error handling if write of a thread failed. all data needs to be deleted and size update reverted
                ld_logger->error("{}() Writing thread {} did not write anything. NO ACTION WAS DONE", __func__, i);
            } else
                write_size += *thread_ret_size;
            ABT_eventual_free(&eventuals[i]);
            ret = ABT_thread_join(threads[i]);
            if (ret != 0) {
                ld_logger->error("{}() Unable to ABT_thread_join()", __func__);
                return -1;
            }
            ret = ABT_thread_free(&threads[i]);
            if (ret != 0) {
                ld_logger->error("{}() Unable to ABT_thread_free()", __func__);
                return -1;
            }
        }
        return write_size;
    }
    return (reinterpret_cast<decltype(&pwrite)>(libc_pwrite))(fd, buf, count, offset);
}

ssize_t read(int fd, void* buf, size_t count) {
    init_passthrough_if_needed();
    if (ld_is_env_initialized() && file_map.exist(fd)) {
        ld_logger->trace("{}() called with fd {}", __func__, fd);
        return pread(fd, buf, count, 0);
    }
    return (reinterpret_cast<decltype(&read)>(libc_read))(fd, buf, count);
}

ssize_t pread(int fd, void* buf, size_t count, off_t offset) {
    init_passthrough_if_needed();
    if (ld_is_env_initialized() && file_map.exist(fd)) {
        ld_logger->trace("{}() called with fd {}", __func__, fd);
        auto adafs_fd = file_map.get(fd);
        auto path = adafs_fd->path();
        auto read_size = static_cast<size_t>(0);
        auto err = 0;

        // Collect all chunk ids within count that have the same destination so that those are send in one rpc bulk transfer
        auto chunk_n = static_cast<size_t>(ceil(
                count / static_cast<float>(CHUNKSIZE))); // get number of chunks needed for writing
        auto chnk_id_start = offset / CHUNKSIZE;
        vector<unsigned long> dest_idx{}; // contains the recipient ids, used to access the dest_ids map
        map<unsigned long, vector<unsigned long>> dest_ids{}; // contains the chnk ids (value list) per recipient (key)
        for (unsigned long i = 0; i < chunk_n; i++) {
            auto chnk_id = i + chnk_id_start;
            auto recipient = get_rpc_node(path + fmt::FormatInt(chnk_id).str());
            if (dest_ids.count(recipient) == 0) {
                dest_ids.insert(make_pair(recipient, vector<unsigned long>{chnk_id}));
                dest_idx.push_back(recipient);
            } else
                dest_ids[recipient].push_back(chnk_id);
        }

        // Create an Argobots thread per destination, fill an appropriate struct with its destination chunk ids
        ABT_xstream xstream;
        ABT_pool pool;
        auto ret = ABT_xstream_self(&xstream);
        if (ret != 0) {
            ld_logger->error("{}() Unable to get self xstream. Is Argobots initialized?", __func__);
            return -1;
        }
        ret = ABT_xstream_get_main_pools(xstream, 1, &pool);
        if (ret != 0) {
            ld_logger->error("{}() Unable to get main pools from ABT xstream", __func__);
            return -1;
        }
        auto dest_n = dest_idx.size();
        vector<ABT_thread> threads(dest_n);
        vector<ABT_eventual> eventuals(dest_n);
        vector<struct read_args*> thread_args(dest_n);
        for (unsigned long i = 0; i < dest_n; i++) {
            ABT_eventual_create(sizeof(size_t), &eventuals[i]);
            struct read_args args = {
                    path, // path
                    count, // total size to read
                    0, // reading offset only for the first chunk
                    buf, // pointer to write buffer
                    dest_ids[dest_idx[i]], // pointer to list of chunk ids that all go to the same destination
                    &eventuals[i], // pointer to an eventual which has allocated memory for storing the written size
            };
            if (i == 0)
                args.in_offset = offset % CHUNKSIZE;
            thread_args[i] = &args;
            ABT_thread_create(pool, rpc_send_read_abt, &(*thread_args[i]), ABT_THREAD_ATTR_NULL, &threads[i]);
        }

        for (unsigned long i = 0; i < dest_n; i++) {
            size_t* thread_ret_size;
            ABT_eventual_wait(eventuals[i], (void**) &thread_ret_size);
            if (thread_ret_size == nullptr || *thread_ret_size == 0) {
                err = -1;
                ld_logger->error("{}() Reading thread {} did not read anything. NO ACTION WAS DONE", __func__, i);
            } else
                read_size += *thread_ret_size;
            ABT_eventual_free(&eventuals[i]);
            ret = ABT_thread_join(threads[i]);
            if (ret != 0) {
                ld_logger->error("{}() Unable to ABT_thread_join()", __func__);
                err = -1;
            }
            ret = ABT_thread_free(&threads[i]);
            if (ret != 0) {
                ld_logger->error("{}() Unable to ABT_thread_free()", __func__);
                err = -1;
            }
        }
        // XXX check how much we need to deal with the read_size
        return err == 0 ? read_size : 0;
    }
    return (reinterpret_cast<decltype(&pread)>(libc_pread))(fd, buf, count, offset);
}

ssize_t pread64(int fd, void* buf, size_t nbyte, __off64_t offset) {
    init_passthrough_if_needed();
    if (ld_is_env_initialized() && file_map.exist(fd)) {
//        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return size written
        return 0; // TODO
    }
    return (reinterpret_cast<decltype(&pread64)>(libc_pread64))(fd, buf, nbyte, offset);
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
    if (ld_is_env_initialized() && file_map.exist(oldfd)) {
// Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&dup)>(libc_dup))(oldfd);
}

int dup2(int oldfd, int newfd) __THROW {
    init_passthrough_if_needed();
    if (ld_is_env_initialized() && (file_map.exist(oldfd) || file_map.exist(newfd))) {
// Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&dup2)>(libc_dup2))(oldfd, newfd);
}