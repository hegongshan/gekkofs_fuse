//
// Created by evie on 11/15/17.
//

#ifndef IFS_GLOBAL_DEFS_HPP
#define IFS_GLOBAL_DEFS_HPP

// These definitions set the RPC's identity and which handler the receiver end should use
#define IPC_FS_CONFIG_TAG "ipc_srv_fs_config"
#define RPC_MINIMAL_TAG "rpc_minimal"
#define RPC_OPEN_TAG "rpc_srv_open"
#define RPC_STAT_TAG "rpc_srv_stat"
#define RPC_UNLINK_TAG "rpc_srv_unlink"
#define RPC_UPDATE_METADENTRY_TAG "rpc_srv_update_metadentry"
#define RPC_UPDATE_METADENTRY_SIZE_TAG "rpc_srv_update_metadentry_size"
#define RPC_WRITE_DATA_TAG "rpc_srv_write_data"
#define RPC_READ_DATA_TAG "rpc_srv_read_data"

#endif //IFS_GLOBAL_DEFS_HPP
