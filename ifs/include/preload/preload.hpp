//
// Created by evie on 7/21/17.
//

#ifndef IOINTERCEPT_PRELOAD_HPP
#define IOINTERCEPT_PRELOAD_HPP

#include <memory>
#include <sys/statfs.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>

#include "../../configure.hpp"

extern "C" {
#include <abt.h>
#include <abt-snoozer.h>
#include <mercury_types.h>
#include <mercury_proc_string.h>
#include <margo.h>
}

#include <preload/open_file_map.hpp>
#include <preload/preload_util.hpp>

struct FsConfig {
    // configurable metadata
    bool atime_state;
    bool mtime_state;
    bool ctime_state;
    bool uid_state;
    bool gid_state;
    bool inode_no_state;
    bool link_cnt_state;
    bool blocks_state;

    uid_t uid;
    gid_t gid;

    std::string mountdir;
    std::string rootdir;
};
extern shared_ptr<struct FsConfig> fs_config;

extern FILE* debug_fd;

#define DAEMON_DEBUG(fd, fmt, ...) \
            do { if (LOG_PRELOAD_DEBUG) fprintf(fd, "[" __DATE__ ":" __TIME__ "] " fmt, ##__VA_ARGS__); fflush(fd); } while (0)
#define DAEMON_DEBUG0(fd, fmt) \
            do { if (LOG_PRELOAD_DEBUG) fprintf(fd, "[" __DATE__ ":" __TIME__ "] " fmt); fflush(fd); } while (0)


bool init_ld_argobots();

void register_client_ipcs();

bool init_ipc_client();

hg_class_t* ld_mercury_ipc_class();

hg_context_t* ld_mercury_ipc_context();

margo_instance_id ld_margo_ipc_id();

hg_addr_t daemon_addr();

void init_passthrough_if_needed();

void init_preload(void) __attribute__((constructor));

void destroy_preload(void) __attribute__((destructor));

#endif //IOINTERCEPT_PRELOAD_HPP
