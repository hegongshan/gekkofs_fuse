//
// Created by evie on 9/7/17.
//

#ifndef IFS_C_METADENTRY_HPP
#define IFS_C_METADENTRY_HPP

#include "../../../main.hpp"
#include "../rpc_types.hpp"

void send_minimal_rpc(void* arg);

int rpc_send_create_node(const size_t recipient, const mode_t mode);

#endif //IFS_C_METADENTRY_HPP
