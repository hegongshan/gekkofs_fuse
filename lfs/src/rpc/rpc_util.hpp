//
// Created by evie on 6/21/17.
//

#ifndef LFS_RPC_UTIL_HPP
#define LFS_RPC_UTIL_HPP

#include "../main.hpp"

bool init_argobots();

void destroy_argobots();

bool init_rpc_server();

void register_server_rpcs(hg_class_t* hg_class);

void destroy_rpc_server();

bool init_rpc_client();

void register_client_rpcs(hg_class_t* hg_class);

void destroy_rpc_client();

#endif //LFS_RPC_UTIL_HPP
