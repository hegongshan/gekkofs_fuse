
#ifndef IFS_ADAFS_DAEMON_HPP
#define IFS_ADAFS_DAEMON_HPP

#include "../../main.hpp"

bool init_environment();
void destroy_enviroment();

bool init_ipc_server();
bool init_rpc_server();

void register_server_rpcs(margo_instance_id mid);

std::string daemon_register_path();

bool register_daemon_proc();

bool deregister_daemon_proc();

#endif //IFS_ADAFS_DAEMON_HPP
