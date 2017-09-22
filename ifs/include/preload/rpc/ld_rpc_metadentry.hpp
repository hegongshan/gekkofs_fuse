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

std::string rpc_send_get_attr(const size_t recipient, const std::string& path);

int rpc_send_remove_node(const hg_id_t rpc_remove_node_id, const size_t recipient, const std::string& path);

#endif //IFS_PRELOAD_C_METADENTRY_HPP
