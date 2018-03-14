
#ifndef IFS_PRELOAD_UTIL_HPP
#define IFS_PRELOAD_UTIL_HPP

#include <preload/preload.hpp>
#include <preload/open_file_map.hpp>
// third party libs
#include <extern/spdlog/spdlog.h>
#include <extern/lrucache/LRUCache11.hpp>
#include <string>

// TODO singleton this stuff away
// The contents of FsConfig or only set once when set up
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

    // rpc infos
    std::map<uint64_t, std::string> hosts;
    std::map<std::string, std::string> sys_hostfile;
    uint64_t host_id; // my host number
    size_t host_size;
    std::string rpc_port;
};
// Used to bundle metadata into one place
struct Metadentry {
    time_t atime;
    time_t mtime;
    time_t ctime;
    uid_t uid;
    gid_t gid;
    mode_t mode;
    uint64_t inode_no;
    nlink_t link_count;
    off_t size;
    blkcnt_t blocks;

    std::string path;
};
struct MetadentryUpdateFlags {
    bool atime = false;
    bool mtime = false;
    bool ctime = false;
    bool uid = false;
    bool gid = false;
    bool mode = false;
    bool inode_no = false;
    bool link_count = false;
    bool size = false;
    bool blocks = false;
    bool path = false;
};

// file map instance for mapping paths to file descriptors
extern OpenFileMap file_map;
// Margo instances
extern margo_instance_id ld_margo_ipc_id;
extern margo_instance_id ld_margo_rpc_id;
// local daemon address for margo ipc client
extern hg_addr_t daemon_svr_addr;
// IPC IDs
extern hg_id_t ipc_minimal_id;
extern hg_id_t ipc_config_id;
extern hg_id_t ipc_mk_node_id;
extern hg_id_t ipc_access_id;
extern hg_id_t ipc_stat_id;
extern hg_id_t ipc_rm_node_id;
extern hg_id_t ipc_update_metadentry_id;
extern hg_id_t ipc_get_metadentry_size_id;
extern hg_id_t ipc_update_metadentry_size_id;
extern hg_id_t ipc_write_data_id;
extern hg_id_t ipc_read_data_id;
// RPC IDs
extern hg_id_t rpc_minimal_id;
extern hg_id_t rpc_mk_node_id;
extern hg_id_t rpc_stat_id;
extern hg_id_t rpc_access_id;
extern hg_id_t rpc_rm_node_id;
extern hg_id_t rpc_update_metadentry_id;
extern hg_id_t rpc_get_metadentry_size_id;
extern hg_id_t rpc_update_metadentry_size_id;
extern hg_id_t rpc_write_data_id;
extern hg_id_t rpc_read_data_id;
// fs_config is set ONCE in the beginning. It shall not be modified afterwards
extern std::shared_ptr<struct FsConfig> fs_config;
// global logger instance
extern std::shared_ptr<spdlog::logger> ld_logger;
// rpc address cache
typedef lru11::Cache<uint64_t, hg_addr_t> KVCache;
extern KVCache rpc_address_cache;
// file descriptor index validation flag
extern std::atomic<bool> fd_validation_needed;
// thread pool
extern ABT_pool io_pool;
extern std::vector<ABT_xstream> io_streams;

// function definitions

int generate_fd_idx();

int get_fd_idx();

bool is_fs_path(const char* path);

// TODO template these two suckers
int db_val_to_stat(std::string path, std::string db_val, struct stat& attr);

int db_val_to_stat64(std::string path, std::string db_val, struct stat64& attr);

int getProcIdByName(std::string procName);

std::string daemon_register_path(int pid);

bool get_daemon_auxiliaries();

bool read_system_hostfile();

bool get_addr_by_hostid(uint64_t hostid, hg_addr_t& svr_addr);

bool is_local_op(size_t recipient);

template<typename T>
hg_return margo_create_wrap(hg_id_t ipc_id, hg_id_t rpc_id, const T&, hg_handle_t& handle,
                            hg_addr_t& svr_addr, bool force_rpc);


#endif //IFS_PRELOAD_UTIL_HPP
