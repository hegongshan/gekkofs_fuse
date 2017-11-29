
#ifndef IFS_DATA_HPP
#define IFS_DATA_HPP

#include "../../main.hpp"
#include <preload/preload_util.hpp>

std::string path_to_fspath(const std::string& path);

int init_chunk_space(const std::string& path);

int destroy_chunk_space(const std::string& path);

int read_file(const std::string& path, rpc_chnk_id_t chnk_id, size_t size, off_t off, char* buf, size_t& read_size);

int write_file(const std::string& path, const char* buf, rpc_chnk_id_t chnk_id, size_t size, off_t off,
               bool append, off_t updated_size, size_t& write_size);

int write_chunks(const std::string& path, const std::vector<void*>& buf_ptrs, const std::vector<hg_size_t>& buf_sizes,
                 const off_t offset, size_t& write_size);

int read_chunks(const std::string& path, const std::vector<void*>& buf_ptrs, const std::vector<hg_size_t>& buf_sizes,
                size_t& read_size);

#endif //IFS_DATA_HPP
