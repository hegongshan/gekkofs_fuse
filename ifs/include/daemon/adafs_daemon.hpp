
#ifndef IFS_ADAFS_DAEMON_HPP
#define IFS_ADAFS_DAEMON_HPP

#include "../../main.hpp"

void init_environment();
void destroy_enviroment();

bool init_ipc_server();
bool init_rpc_server();

void register_server_rpcs(margo_instance_id mid);

#endif //IFS_ADAFS_DAEMON_HPP
