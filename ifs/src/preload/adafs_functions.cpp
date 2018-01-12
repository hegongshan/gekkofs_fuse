#include <preload/adafs_functions.hpp>
#include <preload/rpc/ld_rpc_metadentry.hpp>
#include <preload/rpc/ld_rpc_data_ws.hpp>

using namespace std;


int adafs_open(const std::string& path, mode_t mode, int flags) {
    auto err = 1;
    auto fd = file_map.add(path, (flags & O_APPEND) != 0);
    // TODO look up if file exists configurable
    if (flags & O_CREAT)
        err = rpc_send_open(path, mode, flags);
    else
        err = 0; //TODO default if no o_creat flag, assume file exists. This should be an rpc to see if file is there
    if (err == 0)
        return fd;
    else {
        file_map.remove(fd);
        return -1;
    }
}

int adafs_access(const std::string& path, const mode_t mode) {
    auto err = rpc_send_access(path, mode);
    return err; // XXX for any error, i.e., at least one permission bit is denied or object does not exists. 0 for success
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

ssize_t adafs_pread_ws(int fd, void* buf, size_t count, off_t offset) {
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

ssize_t adafs_pwrite_ws(int fd, const void* buf, size_t count, off_t offset) {
    auto adafs_fd = file_map.get(fd);
    auto path = make_shared<string>(adafs_fd->path());
    auto append_flag = adafs_fd->append_flag();
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