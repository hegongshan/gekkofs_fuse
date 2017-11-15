
#ifndef IFS_PRELOAD_UTIL_HPP
#define IFS_PRELOAD_UTIL_HPP

#include <preload/preload.hpp>
// third party libs
#include <extern/spdlog/spdlog.h>
#include <extern/lrucache/LRUCache11.hpp>
#include <string>

using namespace std;


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

// Margo instances
extern margo_instance_id ld_margo_ipc_id;
extern margo_instance_id ld_margo_rpc_id;
// local daemon address for margo ipc client
extern hg_addr_t daemon_svr_addr;
// IPC IDs
extern hg_id_t ipc_minimal_id;
extern hg_id_t ipc_config_id;
extern hg_id_t ipc_open_id;
extern hg_id_t ipc_stat_id;
extern hg_id_t ipc_unlink_id;
extern hg_id_t ipc_update_metadentry_id;
extern hg_id_t ipc_update_metadentry_size_id;
extern hg_id_t ipc_write_data_id;
extern hg_id_t ipc_read_data_id;
// RPC IDs
extern hg_id_t rpc_minimal_id;
extern hg_id_t rpc_open_id;
extern hg_id_t rpc_stat_id;
extern hg_id_t rpc_unlink_id;
extern hg_id_t rpc_update_metadentry_id;
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

bool is_fs_path(const char* path);

// TODO template these two suckers
int db_val_to_stat(std::string path, std::string db_val, struct stat& attr);

int db_val_to_stat64(std::string path, std::string db_val, struct stat64& attr);

int getProcIdByName(std::string procName);

bool get_addr_by_hostid(uint64_t hostid, hg_addr_t& svr_addr);

size_t get_rpc_node(const std::string& to_hash);

bool is_local_op(size_t recipient);

hg_return margo_create_wrap(hg_id_t ipc_id, hg_id_t rpc_id, const std::string& path, hg_handle_t& handle,
                            hg_addr_t& svr_addr);

#endif //IFS_PRELOAD_UTIL_HPP
