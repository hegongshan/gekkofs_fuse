
#ifndef FS_CONFIGURE_H
#define FS_CONFIGURE_H

// To enabled logging for daemon
#define LOG_INFO
//#define LOG_DEBUG
//#define LOG_TRACE
#define LOG_DAEMON_PATH "/tmp/adafs_daemon.log"

// Enable logging for preload
#define LOG_PRELOAD_INFO
//#define LOG_PRELOAD_DEBUG
//#define LOG_PRELOAD_TRACE
#define LOG_PRELOAD_PATH "/tmp/adafs_preload.log"

// If ACM time should be considered
#define ACMtime //unused
#define BLOCKSIZE 4 // in kilobytes
#define CHUNKSIZE 400 // in bytes

// What metadata is used TODO this has to be parametrized or put into a configuration file
#define MDATA_USE_ATIME false
#define MDATA_USE_MTIME false
#define MDATA_USE_CTIME false
#define MDATA_USE_UID false
#define MDATA_USE_GID false
#define MDATA_USE_INODE_NO false
#define MDATA_USE_LINK_CNT false
#define MDATA_USE_BLOCKS false
#define MDATA_USE_SIZE true // XXX to be added in ADAFS_DATA. currently on by default

// should permissions be checked when access() is called or discarded (disabled by default)
//#define CHECK_ACCESS
// If access permissions should be checked while opening a file (disabled by default)
//#define CHECK_ACCESS_DURING_OPEN
// If disabled, a file or directory is always presumed to be there (even if it is not). No check is executed (enabled by default)
#define DO_LOOKUP

// Write-ahead logging of rocksdb
//#define KV_WOL
// Optimize Key-Value store. Eventually, different modes will be available for different workloads. TODO
//#define KV_OPTIMIZE
// Optimize Key-Value store for tmpfs/ramdisk usage
#define KV_OPTIMIZE_RAMDISK

// RPC configuration
#define RPCPORT 4433
#define RPC_TRIES 3
// rpc timeout to try again in milliseconds
#define RPC_TIMEOUT 180000
// enables timing of sending rpcs
//#define MARGO_FORWARD_TIMER
// sets the threshold in milliseconds when a log entry should be created
#define MARGO_FORWARD_TIMER_THRESHOLD 1000

// Set a hostname suffix when a connection is built. E.g., "-ib" to use Infiniband
#define HOSTNAME_SUFFIX ""
//#define MARGODIAG // enables diagnostics of margo (printed after shutting down

// Debug configurations
//#define RPC_TEST //unused

#endif //FS_CONFIGURE_H
