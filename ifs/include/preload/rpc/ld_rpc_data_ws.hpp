
#ifndef IFS_PRELOAD_C_DATA_WS_HPP
#define IFS_PRELOAD_C_DATA_WS_HPP

extern "C" {
#include <abt.h>
#include <abt-snoozer.h>
#include <mercury_types.h>
#include <mercury_proc_string.h>
#include <margo.h>
}

#include <global/rpc/rpc_types.hpp>
#include <preload/preload.hpp>

#include <iostream>

struct write_args {
    std::shared_ptr<std::string> path;
    size_t total_chunk_size;
    size_t in_size;
    off_t in_offset;
    const void* buf;
    size_t chnk_start;
    size_t chnk_end;
    std::vector<unsigned long>* chnk_ids;
    size_t recipient;
    ABT_eventual eventual;
};

struct read_args {
    std::shared_ptr<std::string> path;
    size_t total_chunk_size;
    size_t in_size;
    off_t in_offset;
    void* buf;
    size_t chnk_start;
    size_t chnk_end;
    std::vector<unsigned long>* chnk_ids;
    size_t recipient;
    ABT_eventual eventual;
};

ssize_t rpc_send_write(const std::string& path, const void* buf, const bool append_flag, const off64_t in_offset,
                       const size_t write_size, const int64_t updated_metadentry_size);

ssize_t rpc_send_read(const std::string& path, void* buf, const off64_t offset, const size_t read_size);

void rpc_send_write_abt(void* _arg);

void rpc_send_read_abt(void* _arg);

#endif //IFS_PRELOAD_C_DATA_WS_HPP
