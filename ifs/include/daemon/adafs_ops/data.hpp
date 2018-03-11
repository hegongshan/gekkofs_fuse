
#ifndef IFS_DATA_HPP
#define IFS_DATA_HPP

#include <daemon/adafs_daemon.hpp>

struct write_chunk_args {
    const std::string* path;
    const char* buf;
    rpc_chnk_id_t chnk_id;
    size_t size;
    off64_t off;
    ABT_eventual eventual;
};
struct read_chunk_args {
    const std::string* path;
    char* buf;
    rpc_chnk_id_t chnk_id;
    size_t size;
    off64_t off;
    ABT_eventual eventual;
};

std::string path_to_fspath(const std::string& path);

int init_chunk_space(const std::string& path);

int destroy_chunk_space(const std::string& path);

void read_file_abt(void* _arg);

void write_file_abt(void* _arg);

int
write_chunks(const std::string& path, const std::vector<char*>& bulk_buf_ptrs, const std::vector<hg_size_t>& chnk_ids,
             const std::vector<hg_size_t>& chnk_sizes, const off64_t offset, size_t& write_size);

int read_chunks(const std::string& path, const std::vector<char*>& bulk_buf_ptrs, const std::vector<uint64_t>& chnk_ids,
                const std::vector<uint64_t>& chnk_sizes, const off64_t offset, size_t& read_size);

#endif //IFS_DATA_HPP
