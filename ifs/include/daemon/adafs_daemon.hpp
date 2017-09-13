//
// Created by evie on 8/31/17.
//

#ifndef IFS_ADAFS_DAEMON_HPP
#define IFS_ADAFS_DAEMON_HPP

#include "../../main.hpp"

void daemon_loop(void* arg);
void run_daemon();

void init_environment();
void destroy_enviroment();

bool init_argobots();
void destroy_argobots();
bool init_rpc_server();

void register_server_rpcs();

bool init_rpc_client();

void register_client_rpcs();


#endif //IFS_ADAFS_DAEMON_HPP
