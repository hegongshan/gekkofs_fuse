//
// Created by evie on 3/17/17.
//

#ifndef FS_CONFIGURE_H
#define FS_CONFIGURE_H

// To enabled logging with info level
//#define LOG_INFO
#define LOG_DEBUG
//#define LOG_TRACE
#define LOG_DAEMON_PATH "/tmp/adafs_daemon.log"

// Enable logging for daemon
#define LOG_PRELOAD_DEBUG 1
#define LOG_PRELOAD_TRACE 1
#define LOG_PRELOAD_PATH "/tmp/adafs_preload.log"

// If ACM time should be considered
#define ACMtime //unused
#define BLOCKSIZE 4 // in kilobytes

// If access permissions should be checked while opening a file
//#define CHECK_ACCESS //unused

// Write-ahead logging of rocksdb
//#define RDB_WOL

// RPC configuration
#define RPCPORT 4433
#define RPC_TIMEOUT 150000
#define RPC_PROTOCOL "bmi+tcp"

// Debug configurations
//#define RPC_TEST //unused

// Using Margo for IPC or raw sockets
#define MARGOIPC

#endif //FS_CONFIGURE_H
