
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

// XXX these two structs can be merged. How to deal with const void* then?
struct write_args {
    std::shared_ptr<std::string> path;
    size_t in_size;
    off_t in_offset;
    const void* buf;
    size_t chnk_start;
    std::vector<unsigned long>* chnk_ids;
    size_t recipient;
    ABT_eventual eventual;
    ABT_barrier barrier;
};

struct read_args {
    std::shared_ptr<std::string> path;
    size_t in_size;
    off_t in_offset;
    void* buf;
    size_t chnk_start;
    std::vector<unsigned long>* chnk_ids;
    size_t recipient;
    ABT_eventual eventual;
    ABT_barrier barrier;
};

void rpc_send_write_abt(void* _arg);

void rpc_send_read_abt(void* _arg);

#endif //IFS_PRELOAD_C_DATA_WS_HPP
