//
// Created by evie on 3/17/17.
//

#ifndef FS_CONFIGURE_H
#define FS_CONFIGURE_H

// To enabled logging with info level
#define LOG_INFO
//#define LOG_DEBUG
//#define LOG_TRACE
#define LOG_PATH "/tmp/adafs.log"

// Enable logging for daemon
#define LOG_DAEMON_DEBUG 1
#define LOG_DAEMON_PATH "/tmp/adafs_daemon.log"

// If ACM time should be considered
#define ACMtime

// If access permissions should be checked while opening a file
//#define CHECK_ACCESS

// Write-ahead logging of rocksdb
//#define RDB_WOL

// RPC configuration
#define RPCPORT 4433
#define RPC_TIMEOUT 15000

// Debug configurations
//#define RPC_TEST

#endif //FS_CONFIGURE_H
