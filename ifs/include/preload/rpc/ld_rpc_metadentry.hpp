//
// Created by evie on 9/7/17.
//

#ifndef IFS_PRELOAD_C_METADENTRY_HPP
#define IFS_PRELOAD_C_METADENTRY_HPP

#include <preload/preload.hpp>
#include <rpc/rpc_types.hpp>
#include <iostream>

void send_minimal_rpc(const hg_id_t minimal_id);

int rpc_send_create_node(const hg_id_t rpc_create_node_id, const size_t recipient, const std::string& path,
                         const mode_t mode);

int
rpc_send_get_attr(const hg_id_t rpc_get_attr_id, const size_t recipient, const std::string& path, std::string& attr);

int rpc_send_remove_node(const hg_id_t rpc_remove_node_id, const size_t recipient, const std::string& path);

int rpc_send_update_metadentry(const hg_id_t ipc_update_metadentry_id, const hg_id_t rpc_update_metadentry_id,
                               const string& path, const Metadentry& md, const MetadentryUpdateFlags& md_flags);

int rpc_send_update_metadentry_size(const hg_id_t ipc_update_metadentry_size_id,
                                    const hg_id_t rpc_update_metadentry_size_id, const string& path, const off_t size);

#endif //IFS_PRELOAD_C_METADENTRY_HPP
