#ifndef IFS_GLOBAL_DEFS_HPP
#define IFS_GLOBAL_DEFS_HPP

// These constexpr set the RPC's identity and which handler the receiver end should use
namespace hg_tag {
    constexpr auto fs_config = "ipc_srv_fs_config";
    constexpr auto minimal = "rpc_minimal";
    constexpr auto create = "rpc_srv_mk_node";
    constexpr auto access = "rpc_srv_access";
    constexpr auto stat = "rpc_srv_stat";
    constexpr auto remove = "rpc_srv_rm_node";
    constexpr auto update_metadentry = "rpc_srv_update_metadentry";
    constexpr auto update_metadentry_size = "rpc_srv_update_metadentry_size";
    constexpr auto write_data = "rpc_srv_write_data";
    constexpr auto read_data = "rpc_srv_read_data";
}

#endif //IFS_GLOBAL_DEFS_HPP
