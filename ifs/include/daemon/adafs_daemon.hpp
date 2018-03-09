
#ifndef IFS_ADAFS_DAEMON_HPP
#define IFS_ADAFS_DAEMON_HPP

// std libs
#include <string>
#include <iostream>
#include <cstdint>
#include <cerrno>
#include <unordered_map>
#include <thread>
#include <map>

// adafs config
#include <global/configure.hpp>
#include <global/global_defs.hpp>
// boost libs
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
// third party libs
#include <extern/spdlog/spdlog.h>
#include <extern/spdlog/fmt/fmt.h>
// rocksdb
#include <rocksdb/db.h>
#include <rocksdb/slice.h>
#include <rocksdb/options.h>
#include <rocksdb/utilities/transaction.h>
#include <rocksdb/utilities/optimistic_transaction_db.h>
#include <rocksdb/write_batch.h>
// margo
extern "C" {
#include <abt.h>
#include <abt-snoozer.h>
#include <mercury.h>
#include <margo.h>
}
// adafs
#include <daemon/classes/fs_data.hpp>
#include <daemon/classes/rpc_data.hpp>

namespace bfs = boost::filesystem;

#define INVALID_INODE static_cast<ino_t>(0)
#define ADAFS_DATA (static_cast<FsData*>(FsData::getInstance()))
#define RPC_DATA (static_cast<RPCData*>(RPCData::getInstance()))

bool init_environment();
void destroy_enviroment();

bool init_io_tasklet_pool();
bool init_ipc_server();
bool init_rpc_server();

void register_server_rpcs(margo_instance_id mid);

std::string daemon_register_path();

bool register_daemon_proc();

bool deregister_daemon_proc();

std::string get_my_hostname(bool short_hostname);

#endif //IFS_ADAFS_DAEMON_HPP
