
#ifndef IFS_DATA_HPP
#define IFS_DATA_HPP

#include "../../main.hpp"

std::string path_to_fspath(const std::string& path);

int init_chunk_space(const std::string& path);

int destroy_chunk_space(const std::string& path);

int read_file(char* buf, size_t& read_size, const std::string& path, const size_t size, const off_t off);

int write_file(const std::string& path, const char* buf, size_t& write_size, const size_t size, const off_t off,
               const bool append, const off_t updated_size);

#endif //IFS_DATA_HPP
