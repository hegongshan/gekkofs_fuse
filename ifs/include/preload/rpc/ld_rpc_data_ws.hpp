
#ifndef IFS_PRELOAD_C_DATA_WS_HPP
#define IFS_PRELOAD_C_DATA_WS_HPP

ssize_t rpc_send_write(const std::string& path, const void* buf, const bool append_flag, const off64_t in_offset,
                       const size_t write_size, const int64_t updated_metadentry_size);

ssize_t rpc_send_read(const std::string& path, void* buf, const off64_t offset, const size_t read_size);

int rpc_send_trunc_data(const std::string& path, size_t current_size, size_t new_size);

#endif //IFS_PRELOAD_C_DATA_WS_HPP
