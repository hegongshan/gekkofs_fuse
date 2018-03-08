
#include <daemon/adafs_ops/data.hpp>

using namespace std;

std::string path_to_fspath(const std::string& path) {
    // root path is absolute as is the path comes in here which is hierarchically under root_path
    // XXX check if this can be done easier
    string fs_path;
    set_difference(path.begin(), path.end(), ADAFS_DATA->mountdir().begin(), ADAFS_DATA->mountdir().end(),
                   std::back_inserter(fs_path));
    if (fs_path.at(0) == '/') {
        fs_path = fs_path.substr(1, fs_path.size());
    }
    // replace / with : to avoid making a bunch of mkdirs to store the data in the underlying fs. XXX Can this be done with hashing?
    replace(fs_path.begin(), fs_path.end(), '/', ':');
    return fs_path;
}

/**
 * Creates the directory in the chunk dir for a file to hold data
 * @param inode
 * @return
 */
// XXX this might be just a temp function as long as we don't use chunks
// XXX this function creates not only the chunk folder but also a single file which holds the data of the 'real' file
int init_chunk_space(const std::string& path) {
    auto fs_path = path_to_fspath(path);

    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= fs_path;

    // create chunk dir
    bfs::create_directories(chnk_path);

    // XXX create temp big file. remember also to modify the return value
    chnk_path /= "data"s;
    bfs::ofstream ofs{chnk_path};

//    return static_cast<int>(bfs::exists(chnk_path));
    return 0;
}
/**
 * Remove the directory in the chunk dir of a file.
 * @param inode
 * @return
 */
// XXX this might be just a temp function as long as we don't use chunks
int destroy_chunk_space(const std::string& path) {
    auto fs_path = path_to_fspath(path);

    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= fs_path;

    // create chunk dir
    bfs::remove_all(chnk_path);

//    return static_cast<int>(!bfs::exists(chnk_path));
    return 0;
}

/**
 * Used by an argobots threads. Argument args has the following fields:
 * const std::string* path;
   const char* buf;
   const rpc_chnk_id_t* chnk_id;
   size_t size;
   off64_t off;
   ABT_eventual* eventual;
 * Because abt-io uses tasklets, calling threads are suspended for the time of the I/O, allowing other threads to do
 * work in the same ES. This can mean multiple write_file_abt() calls or other RPC calls
 * @return written_size<size_t> is put into eventual and returned that way
 */
void write_file_abt(void* _arg) {
    size_t write_size = 0;
    // Unpack args
    auto* arg = static_cast<struct write_chunk_args*>(_arg);
    auto fs_path = path_to_fspath(*arg->path);
    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= fs_path;
    bfs::create_directories(chnk_path);
    chnk_path /= fmt::FormatInt(*arg->chnk_id).c_str();
    // open file
    int fd = open(chnk_path.c_str(), O_WRONLY | O_CREAT, 0640);
    if (fd < 0) {
        write_size = static_cast<size_t>(EIO);
        ABT_eventual_set(*(arg->eventual), &write_size, sizeof(size_t));
        return;
    }
    // write file
    auto err = static_cast<size_t>(pwrite(fd, arg->buf, arg->size, arg->off));
    if (err < 0) {
        ADAFS_DATA->spdlogger()->error("{}() Error {} while pwriting file {} chunk_id {} size {} off {}", __func__,
                                       strerror(errno), chnk_path.c_str(), *arg->chnk_id, arg->size, arg->off);
    } else {
        write_size = static_cast<size_t>(err); // This is cast safe
    }
    // file is closed
    close(fd);
    ABT_eventual_set(*(arg->eventual), &write_size, sizeof(size_t));
}

int write_chunks(const string& path, const vector<void*>& buf_ptrs, const vector<hg_size_t>& buf_sizes,
                 const off64_t offset, size_t& write_size) {
    write_size = 0;
    // buf sizes also hold chnk ids. we only want to keep calculate the actual chunks
    auto chnk_n = static_cast<unsigned int>(buf_sizes.size() / 2); // Case-safe: There never are so many chunks at once
    vector<ABT_eventual> eventuals(chnk_n);
    vector<unique_ptr<struct write_chunk_args>> thread_args(chnk_n);

    for (unsigned int i = 0; i < chnk_n; i++) {
        auto chnk_id = static_cast<size_t*>(buf_ptrs[i]);
        auto chnk_ptr = static_cast<char*>(buf_ptrs[i + chnk_n]);
        auto chnk_size = buf_sizes[i + chnk_n];
        // Starting thread for parallel I/O
        // Note that the parallelism comes from abt-io not from the threads as they are ULTs in a sequential stream
        ABT_eventual_create(sizeof(size_t), &eventuals[i]); // written file return value
        auto args = make_unique<write_chunk_args>();
        args->path = &path;
        args->buf = chnk_ptr;
        args->chnk_id = chnk_id;
        args->size = chnk_size;
        // only the first chunk gets the offset. the chunks are sorted on the client side
        args->off = (i == 0 ? offset : 0);
        args->eventual = &eventuals[i];
        thread_args[i] = std::move(args);
        auto ret = ABT_task_create(RPC_DATA->io_pool(), write_file_abt, &(*thread_args[i]), nullptr);
        if (ret != ABT_SUCCESS) {
            ADAFS_DATA->spdlogger()->error("{}() task create failed", __func__);
        }
    }
    for (unsigned int i = 0; i < chnk_n; i++) {
        size_t* thread_written_size;
        ABT_eventual_wait(eventuals[i], (void**) &thread_written_size);
        if (thread_written_size == nullptr || *thread_written_size == 0) {
            ADAFS_DATA->spdlogger()->error("{}() Writing file thread {} did return nothing. NO ACTION WAS DONE",
                                           __func__, i);
//            // TODO How do we handle already written chunks? Ideally, we would need to remove them after failure.
//            ADAFS_DATA->spdlogger()->error("{}() Writing chunk failed with path {} and id {}. Aborting ...", __func__,
//                                           path, chnk_id);
            write_size = 0;
            return -1;
        } else {
            write_size += *thread_written_size;
        }
        ABT_eventual_free(&eventuals[i]);
    }
    return 0;
}

/**
 *
 * @param path
 * @param chnk_id
 * @param size
 * @param off
 * @param [out] buf
 * @param [out] read_size
 * @return
 */
int read_file(const string& path, const rpc_chnk_id_t chnk_id, const size_t size, const off64_t off, char* buf,
              size_t& read_size) {
    auto fs_path = path_to_fspath(path);
    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= fs_path;
    chnk_path /= fmt::FormatInt(chnk_id).c_str();;

    int fd = open(chnk_path.c_str(), R_OK);
    if (fd < 0)
        return EIO;
    auto err = pread64(fd, buf, size, off);
    if (err < 0) {
        ADAFS_DATA->spdlogger()->error("{}() Error {} while preading file {} chunk_id {} size {} off {}", __func__,
                                       strerror(errno), chnk_path.c_str(), chnk_id, size, off);
        read_size = 0;
    } else
        read_size = static_cast<size_t>(err); // This is cast safe
    close(fd);
    return 0;
}

int read_chunks(const string& path, const off64_t offset, const vector<void*>& buf_ptrs,
                const vector<hg_size_t>& buf_sizes,
                size_t& read_size) {
    read_size = 0;
    // buf sizes also hold chnk ids. we only want to keep calculate the actual chunks
    auto chnk_n = buf_sizes.size() / 2;
    // TODO this can be parallized
    for (size_t i = 0; i < chnk_n; i++) {
        auto chnk_id = *(static_cast<size_t*>(buf_ptrs[i]));
        auto chnk_ptr = static_cast<char*>(buf_ptrs[i + chnk_n]);
        auto chnk_size = buf_sizes[i + chnk_n];
        size_t read_chnk_size;
        // read_file but only first chunk can have an offset
        if (read_file(path, chnk_id, chnk_size, (i == 0) ? offset : 0, chnk_ptr, read_chnk_size) != 0) {
            // TODO How do we handle errors?
            ADAFS_DATA->spdlogger()->error("{}() read chunk failed with path {} and id {}. Aborting ...", __func__,
                                           path, chnk_id);
            read_size = 0;
            return -1;
        }
        read_size += read_chnk_size;
    }
    return 0;
}