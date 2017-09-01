//
// Created by evie on 8/31/17.
//

#ifndef IFS_MAIN_HPP
#define IFS_MAIN_HPP

// std libs
#include <string>
#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <thread>

// adafs config
#include "configure.hpp"
// boost libs
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
// third party libs
#include "extern/spdlog/spdlog.h"
#include "extern/spdlog/fmt/fmt.h"
// rocksdb
#include <rocksdb/db.h>
#include <rocksdb/slice.h>
#include <rocksdb/options.h>
#include <rocksdb/utilities/transaction.h>
#include <rocksdb/utilities/optimistic_transaction_db.h>
// margo
extern "C" {
#include <abt.h>
#include <abt-snoozer.h>
//#include <abt-io.h>
//#include <mercury.h>
#include <mercury_types.h>
#include <mercury_proc_string.h>
#include <margo.h>
}
// adafs
#include "include/classes/fs_data.hpp"

namespace bfs = boost::filesystem;

#define ADAFS_DATA (static_cast<FsData*>(FsData::getInstance()))
#define RPC_DATA (static_cast<RPCData*>(RPCData::getInstance()))

#endif //IFS_MAIN_HPP
