#include <preload/adafs_functions.hpp>
#include <preload/rpc/ld_rpc_metadentry.hpp>
#include <preload/rpc/ld_rpc_data_ws.hpp>

using namespace std;


int adafs_open(const std::string& path, mode_t mode, int flags) {
    init_ld_env_if_needed();
    auto err = 1;
    auto fd = file_map.add(path, flags);
    // TODO the open flags should not be in the map just set the pos accordingly
    // TODO look up if file exists configurable
    if (flags & O_CREAT)
        // no access check required here. If one is using our FS they have the permissions.
        err = rpc_send_mk_node(path, mode);
    else {
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
#else
        // file is assumed to be existing, even though it might not
        err = 0;
#endif
    }
    if (err == 0)
        return fd;
    else {
        file_map.remove(fd);
        return -1;
    }
}

int adafs_mk_node(const std::string& path, const mode_t mode) {
    init_ld_env_if_needed();
    return rpc_send_mk_node(path, mode);
}

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
int adafs_stat(const std::string& path, struct stat* buf) {
    init_ld_env_if_needed();
    string attr = ""s;
    auto err = rpc_send_stat(path, attr);
    if (err == 0)
        db_val_to_stat(path, attr, *buf);
    return err;
}

int adafs_stat64(const std::string& path, struct stat64* buf) {
    init_ld_env_if_needed();
    string attr = ""s;
    auto err = rpc_send_stat(path, attr);
    if (err == 0)
        db_val_to_stat64(path, attr, *buf);
    return err;
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

ssize_t adafs_pread_ws(int fd, void* buf, size_t count, off64_t offset) {
    init_ld_env_if_needed();
    auto adafs_fd = file_map.get(fd);
    auto path = make_shared<string>(adafs_fd->path());
    auto read_size = static_cast<size_t>(0);
    auto err = 0;

    // Collect all chunk ids within count that have the same destination so that those are send in one rpc bulk transfer
    auto chnk_start = static_cast<size_t>(offset) / CHUNKSIZE; // first chunk number
    auto chnk_end = (offset + count) / CHUNKSIZE + 1; // last chunk number (right-open) [chnk_start,chnk_end)
    if ((offset + count) % CHUNKSIZE == 0)
        chnk_end--;
    vector<unsigned long> dest_idx{}; // contains the recipient ids, used to access the dest_ids map
    map<unsigned long, vector<unsigned long>> dest_ids{}; // contains the chnk ids (value list) per recipient (key)
    for (unsigned long i = chnk_start; i < chnk_end; i++) {
        auto recipient = get_rpc_node(*path + fmt::FormatInt(i).str());
        if (dest_ids.count(recipient) == 0) {
            dest_ids.insert(make_pair(recipient, vector<unsigned long>{i}));
            dest_idx.push_back(recipient);
        } else
            dest_ids[recipient].push_back(i);
    }

    auto dest_n = dest_idx.size();
    // Create an Argobots thread per destination, fill an appropriate struct with its destination chunk ids
    vector<ABT_xstream> xstreams(dest_n);
    vector<ABT_pool> pools(dest_n);
    int ret;
    for(unsigned long i = 0; i < dest_n; i++) {
        ret = ABT_xstream_create(ABT_SCHED_NULL, &xstreams[i]);
        if (ret != 0) {
            ld_logger->error("{}() Unable to create xstreams, for parallel read", __func__);
            errno = EAGAIN;
            return -1;
        }
        ret = ABT_xstream_get_main_pools(xstreams[i], 1, &pools[i]);
        if (ret != 0) {
            ld_logger->error("{}() Unable to get main pool from xstream", __func__);
            errno = EAGAIN;
            return -1;
        }
    }
    vector<ABT_thread> threads(dest_n);
    vector<ABT_eventual> eventuals(dest_n);
    vector<unique_ptr<struct read_args>> thread_args(dest_n);
    for (unsigned long i = 0; i < dest_n; i++) {
        ABT_eventual_create(sizeof(size_t), &eventuals[i]);
        auto total_chunk_size = dest_ids[dest_idx[i]].size() * CHUNKSIZE;
        if (i == 0) // receiver of first chunk must subtract the offset from first chunk
            total_chunk_size -= offset % CHUNKSIZE;
        if (i == dest_n - 1 && ((offset + count) % CHUNKSIZE) != 0) // receiver of last chunk must subtract
            total_chunk_size -= CHUNKSIZE - ((offset + count) % CHUNKSIZE);
        auto args = make_unique<read_args>();
        args->path = path;
        args->in_size = total_chunk_size;// total size to read
        args->in_offset = offset % CHUNKSIZE;// reading offset only for the first chunk
        args->buf = buf;
        args->chnk_ids = &dest_ids[dest_idx[i]]; // pointer to list of chunk ids that all go to the same destination
        args->chnk_start = chnk_start;
        args->recipient = dest_idx[i];// recipient
        args->eventual = &eventuals[i];// pointer to an eventual which has allocated memory for storing the written size
        thread_args[i] = std::move(args);
        ABT_thread_create(pools[i], rpc_send_read_abt, &(*thread_args[i]), ABT_THREAD_ATTR_NULL, &threads[i]);
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
            ld_logger->warn("{}() Unable to ABT_thread_free()", __func__);
        }
        ret = ABT_xstream_free(&xstreams[i]);
        if (ret != 0) {
            ld_logger->warn("{}() Unable to free xstreams", __func__);
        }
    }
    // XXX check how much we need to deal with the read_size
    // XXX check that we don't try to read past end of the file
    return err == 0 ? read_size : 0;
}

ssize_t adafs_pwrite_ws(int fd, const void* buf, size_t count, off64_t offset) {
    init_ld_env_if_needed();
    auto adafs_fd = file_map.get(fd);
    auto path = make_shared<string>(adafs_fd->path());
    auto append_flag = adafs_fd->get_flag(OpenFile_flags::append);
    int err = 0;
    long updated_size = 0;
    auto write_size = static_cast<size_t>(0);

    err = rpc_send_update_metadentry_size(*path, count, offset, append_flag, updated_size);
    if (err != 0) {
        ld_logger->error("{}() update_metadentry_size failed with err {}", __func__, err);
        return 0; // ERR
    }
    if (append_flag)
        offset = updated_size - count;

    auto chnk_start = static_cast<size_t>(offset) / CHUNKSIZE; // first chunk number
    auto chnk_end = (offset + count) / CHUNKSIZE + 1; // last chunk number (right-open) [chnk_start,chnk_end)
    if ((offset + count) % CHUNKSIZE == 0)
        chnk_end--;

    // Collect all chunk ids within count that have the same destination so that those are send in one rpc bulk transfer
    map<unsigned long, vector<unsigned long>> dest_ids{};
    // contains the recipient ids, used to access the dest_ids map. First idx is chunk with potential offset
    vector<unsigned long> dest_idx{};
    for (auto i = chnk_start; i < chnk_end; i++) {
        auto recipient = get_rpc_node(*path + fmt::FormatInt(i).str());
        if (dest_ids.count(recipient) == 0) {
            dest_ids.insert(make_pair(recipient, vector<unsigned long>{i}));
            dest_idx.push_back(recipient);
        } else
            dest_ids[recipient].push_back(i);
    }
    // Create an Argobots thread per destination, fill an appropriate struct with its destination chunk ids
    auto dest_n = dest_idx.size();
    vector<ABT_xstream> xstreams(dest_n);
    vector<ABT_pool> pools(dest_n);
    int ret;
    for(unsigned long i = 0; i < dest_n; i++) {
        ret = ABT_xstream_create(ABT_SCHED_NULL, &xstreams[i]);
        if (ret != 0) {
            ld_logger->error("{}() Unable to create xstreams, for parallel read", __func__);
            errno = EAGAIN;
            return -1;
        }
        ret = ABT_xstream_get_main_pools(xstreams[i], 1, &pools[i]);
        if (ret != 0) {
            ld_logger->error("{}() Unable to get main pool from xstream", __func__);
            errno = EAGAIN;
            return -1;
        }
    }
    vector<ABT_thread> threads(dest_n);
    vector<ABT_eventual> eventuals(dest_n);
    vector<unique_ptr<struct write_args>> thread_args(dest_n);
    for (unsigned long i = 0; i < dest_n; i++) {
        ABT_eventual_create(sizeof(size_t), &eventuals[i]);
        auto total_chunk_size = dest_ids[dest_idx[i]].size() * CHUNKSIZE;
        if (i == 0) // receiver of first chunk must subtract the offset from first chunk
            total_chunk_size -= offset % CHUNKSIZE;
        if (i == dest_n - 1 && ((offset + count) % CHUNKSIZE) != 0) // receiver of last chunk must subtract
            total_chunk_size -= CHUNKSIZE - ((offset + count) % CHUNKSIZE);
        auto args = make_unique<write_args>();
        args->path = path; // path
        args->in_size = total_chunk_size; // total size to write
        args->in_offset = offset % CHUNKSIZE;// first offset in dest_idx is the chunk with a potential offset
        args->buf = buf;// pointer to write buffer
        args->chnk_start = chnk_start;// append flag when file was opened
        args->chnk_ids = &dest_ids[dest_idx[i]];// pointer to list of chunk ids that all go to the same destination
        args->recipient = dest_idx[i];// recipient
        args->eventual = &eventuals[i];// pointer to an eventual which has allocated memory for storing the written size
        thread_args[i] = std::move(args);
        ld_logger->info("{}() Starting thread with recipient {} and chnk_ids_n {}", __func__, dest_idx[i],
                        dest_ids[dest_idx[i]].size());
        ABT_thread_create(pools[i], rpc_send_write_abt, &(*thread_args[i]), ABT_THREAD_ATTR_NULL, &threads[i]);
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
            ld_logger->warn("{}() Unable to ABT_thread_free()", __func__);
        }
        ret = ABT_xstream_free(&xstreams[i]);
        if (ret != 0) {
            ld_logger->warn("{}() Unable to free xstreams", __func__);
        }
    }
    return write_size;
}