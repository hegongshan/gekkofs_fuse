#include "../../configure_public.hpp"

/**
 * Attention: This whole configfile is not in a final form! This will eventually be a plaintext config file.
 */

#ifndef FS_CONFIGURE_H
#define FS_CONFIGURE_H

// Daemon path to auxiliary files
#define DAEMON_AUX_PATH "/tmp/adafs"

// If ACM time should be considered
#define ACMtime //unused
// XXX Should blocksize and chunksize be merged?
#define BLOCKSIZE 524288 // in bytes 512KB
#define CHUNKSIZE 524288 // in bytes 512KB

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
//#define KV_OPTIMIZE_RAMDISK

// Buffer size for Rocksdb. A high number means that all entries are held in memory.
// However, when full the application blocks until **all** entries are flushed to disk.
//#define KV_WRITE_BUFFER 16384

// Margo configuration

// Number of threads used for concurrent I/O
#define IO_THREADS 8
// Number of threads used for RPC and IPC handlers at the daemon
#define RPC_HANDLER_THREADS 8
#define IPC_HANDLER_THREADS 8
#define RPC_PORT 4433
#define RPC_TRIES 3
// rpc timeout to try again in milliseconds
#define RPC_TIMEOUT 180000
// enables timing of sending rpcs
//#define MARGO_FORWARD_TIMER
// sets the threshold in milliseconds when a log entry should be created
#define MARGO_FORWARD_TIMER_THRESHOLD 1000

// Debug configurations
//#define RPC_TEST //unused

#endif //FS_CONFIGURE_H
