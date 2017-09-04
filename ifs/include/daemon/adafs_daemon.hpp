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
bool init_rpc_client();


#endif //IFS_ADAFS_DAEMON_HPP
