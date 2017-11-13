//
// Created by evie on 7/21/17.
//

#include <preload/preload.hpp>
#include <preload/ipc_types.hpp>
#include <preload/margo_ipc.hpp>
#include <preload/rpc/ld_rpc_metadentry.hpp>
#include <extern/lrucache/LRUCache11.hpp>

#include <dlfcn.h>
#include <cassert>
#include <preload/rpc/ld_rpc_data.hpp>

// TODO my god... someone clean up this mess of a file :_:

static pthread_once_t init_lib_thread = PTHREAD_ONCE_INIT;

enum class Margo_mode {
    RPC, IPC
};
// Mercury/Margo IPC Client
margo_instance_id margo_ipc_id_;
hg_addr_t daemon_svr_addr_ = HG_ADDR_NULL;
// Mercury/Margo RPC Client
margo_instance_id margo_rpc_id_;

// IPC IDs
static hg_id_t minimal_id;
static hg_id_t ipc_config_id;
static hg_id_t ipc_open_id;
static hg_id_t ipc_stat_id;
static hg_id_t ipc_unlink_id;
static hg_id_t ipc_update_metadentry_id;
static hg_id_t ipc_update_metadentry_size_id;
static hg_id_t ipc_write_data_id;
static hg_id_t ipc_read_data_id;
// RPC IDs
static hg_id_t rpc_minimal_id;
static hg_id_t rpc_open_id;
static hg_id_t rpc_stat_id;
static hg_id_t rpc_remove_node_id;
static hg_id_t rpc_update_metadentry_id;
static hg_id_t rpc_update_metadentry_size_id;
static hg_id_t rpc_write_data_id;
static hg_id_t rpc_read_data_id;
// rpc address cache
typedef lru11::Cache<uint64_t, hg_addr_t> KVCache;
KVCache rpc_address_cache_{32768, 4096}; // XXX Set values are not based on anything...

// misc
static std::atomic<bool> is_env_initialized(false);

// external variables
shared_ptr<FsConfig> fs_config;
shared_ptr<spdlog::logger> ld_logger;

// function pointer for preloading
void* libc;

void* libc_open;
//void* libc_open64; //unused
void* libc_fopen; // XXX Does not work with streaming pointers. If used will block forever
void* libc_fopen64; // XXX Does not work with streaming pointers. If used will block forever

//void* libc_creat; //unused
//void* libc_creat64; //unused
void* libc_unlink;

void* libc_close;
//void* libc___close; //unused

void* libc_stat;
void* libc_fstat;
void* libc___xstat;
void* libc___xstat64;
void* libc___fxstat;
void* libc___fxstat64;
void* libc___lxstat;
void* libc___lxstat64;

void* libc_access;

void* libc_puts;

void* libc_write;
void* libc_pwrite;
void* libc_read;
void* libc_pread;
void* libc_pread64;

void* libc_lseek;
//void* libc_lseek64; //unused

void* libc_truncate;
void* libc_ftruncate;

void* libc_dup;
void* libc_dup2;


static OpenFileMap file_map{};

int open(const char* path, int flags, ...) {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list vl;
        va_start(vl, flags);
        mode = static_cast<mode_t>(va_arg(vl, int));
        va_end(vl);
    }
    if (is_env_initialized && is_fs_path(path)) {
        auto err = 1;
        auto fd = file_map.add(path, (flags & O_APPEND) != 0);
        // TODO look up if file exists configurable
        err = rpc_send_open(ipc_open_id, rpc_open_id, path, mode, flags);
        if (err == 0)
            return fd;
        else {
            file_map.remove(fd);
            return -1;
        }
    }
    return (reinterpret_cast<decltype(&open)>(libc_open))(path, flags, mode);
}

int open64(__const char* path, int flags, ...) {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return open(path, flags | O_LARGEFILE, mode);
}

//// TODO This function somehow always blocks forever if one puts anything between the paththru...
//FILE* fopen(const char* path, const char* mode) {
////    init_passthrough_if_needed();
////    DAEMON_DEBUG(debug_fd, "fopen called with path %s\n", path);
//    return (reinterpret_cast<decltype(&fopen)>(libc_fopen))(path, mode);
//}

//// TODO This function somehow always blocks forever if one puts anything between the paththru...
//FILE* fopen64(const char* path, const char* mode) {
////    init_passthrough_if_needed();
////    DAEMON_DEBUG(debug_fd, "fopen64 called with path %s\n", path);
//    return (reinterpret_cast<decltype(&fopen)>(libc_fopen))(path, mode);
//}

#undef creat
int creat(const char* path, mode_t mode) {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {} with mode {}", __func__, path, mode);
    return open(path, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

#undef creat64

int creat64(const char* path, mode_t mode) {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {} with mode {}", __func__, path, mode);
    return open(path, O_CREAT | O_WRONLY | O_TRUNC | O_LARGEFILE, mode);
}

int unlink(const char* path) __THROW {
    init_passthrough_if_needed();
    int err;
//    LD_LOG_DEBUG(debug_fd, "unlink called with path %s\n", path);
    if (is_env_initialized && is_fs_path(path)) {
        if (fs_config->host_size > 1) { // multiple node operation
            auto recipient = get_rpc_node(path);
            if (is_local_op(recipient)) { // local
                err = ipc_send_unlink(path, ipc_unlink_id);
            } else { // remote
                err = rpc_send_remove_node(rpc_remove_node_id, recipient, path);
            }
        } else { // single node operation
            err = ipc_send_unlink(path, ipc_unlink_id);
        }

        return err;
    }
    return (reinterpret_cast<decltype(&unlink)>(libc_unlink))(path);
}

int close(int fd) {
    init_passthrough_if_needed();
    if (is_env_initialized && file_map.exist(fd)) {
        // Currently no call to the daemon is required
        file_map.remove(fd);
        return 0;
    }
    return (reinterpret_cast<decltype(&close)>(libc_close))(fd);
}

int __close(int fd) {
    return close(fd);
}

// TODO combine adafs_stat and adafs_stat64
int adafs_stat(const std::string& path, struct stat* buf) {
    string attr = ""s;
    auto err = rpc_send_stat(ipc_stat_id, rpc_stat_id, path, attr);
    db_val_to_stat(path, attr, *buf);
    return err;
}

int adafs_stat64(const std::string& path, struct stat64* buf) {
    string attr = ""s;
    auto err = rpc_send_stat(ipc_stat_id, rpc_stat_id, path, attr);
    db_val_to_stat64(path, attr, *buf);
    return err;
}

int stat(const char* path, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    if (is_env_initialized && is_fs_path(path)) {
        // TODO call daemon and return
        return adafs_stat(path, buf);
    }
    return (reinterpret_cast<decltype(&stat)>(libc_stat))(path, buf);
}

int fstat(int fd, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with fd {}", __func__, fd);
    if (is_env_initialized && file_map.exist(fd)) {
        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return
        return adafs_stat(path, buf);
    }
    return (reinterpret_cast<decltype(&fstat)>(libc_fstat))(fd, buf);
}

int __xstat(int ver, const char* path, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    if (is_env_initialized && is_fs_path(path)) {
        // TODO call stat
        return adafs_stat(path, buf);
    }
    return (reinterpret_cast<decltype(&__xstat)>(libc___xstat))(ver, path, buf);
}

int __xstat64(int ver, const char* path, struct stat64* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with path {}", __func__, path);
    if (is_env_initialized && is_fs_path(path)) {
        return adafs_stat64(path, buf);
//        // Not implemented
//        return -1;
    }
    return (reinterpret_cast<decltype(&__xstat64)>(libc___xstat64))(ver, path, buf);
}

int __fxstat(int ver, int fd, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with fd {}", __func__, fd);
    if (is_env_initialized && file_map.exist(fd)) {
        // TODO call fstat
        auto path = file_map.get(fd)->path();
        return adafs_stat(path, buf);
    }
    return (reinterpret_cast<decltype(&__fxstat)>(libc___fxstat))(ver, fd, buf);
}

int __fxstat64(int ver, int fd, struct stat64* buf) __THROW {
    init_passthrough_if_needed();
    ld_logger->trace("{}() called with fd {}", __func__, fd);
    if (is_env_initialized && file_map.exist(fd)) {
        // TODO call fstat64
        auto path = file_map.get(fd)->path();
        return adafs_stat64(path, buf);
    }
    return (reinterpret_cast<decltype(&__fxstat64)>(libc___fxstat64))(ver, fd, buf);
}

extern int __lxstat(int ver, const char* path, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    if (is_env_initialized && is_fs_path(path)) {
        // Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&__lxstat)>(libc___lxstat))(ver, path, buf);
}

extern int __lxstat64(int ver, const char* path, struct stat64* buf) __THROW {
    init_passthrough_if_needed();
    if (is_env_initialized && is_fs_path(path)) {
        // Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&__lxstat64)>(libc___lxstat64))(ver, path, buf);
}

int access(const char* path, int mode) __THROW {
    init_passthrough_if_needed();
    if (is_env_initialized && is_fs_path(path)) {
        // TODO
    }
    return (reinterpret_cast<decltype(&access)>(libc_access))(path, mode);
}

int puts(const char* str) {
    return (reinterpret_cast<decltype(&puts)>(libc_puts))(str);
}

ssize_t write(int fd, const void* buf, size_t count) {
    init_passthrough_if_needed();
    if (is_env_initialized && file_map.exist(fd)) {
        // TODO if append flag has been given, set offset accordingly.
        // XXX handle lseek too
        return pwrite(fd, buf, count, 0);
    }
    return (reinterpret_cast<decltype(&write)>(libc_write))(fd, buf, count);
}

ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset) {
    init_passthrough_if_needed();
    if (is_env_initialized && file_map.exist(fd)) {
        auto adafs_fd = file_map.get(fd);
        auto path = adafs_fd->path();
        auto append_flag = adafs_fd->append_flag();
        size_t write_size;
        int err = 0;
        long updated_size = 0;
        /*
         * Update the metadentry size first to prevent two processes to write to the same offset when O_APPEND is given.
         * The metadentry size update is atomic XXX actually not yet. see metadentry.cpp
         */
//        if (append_flag)
            err = rpc_send_update_metadentry_size(ipc_update_metadentry_size_id, rpc_update_metadentry_size_id, path,
                                                  count, append_flag, updated_size);
        if (err != 0) {
            ld_logger->error("{}() update_metadentry_size failed", __func__);
            return 0; // ERR
        }
        err = rpc_send_write(ipc_write_data_id, rpc_write_data_id, path, count, offset, buf, write_size, append_flag,
                             updated_size);
        if (err != 0) {
            ld_logger->error("{}() write failed", __func__);
            return 0;
        }
        return write_size;
    }
    return (reinterpret_cast<decltype(&pwrite)>(libc_pwrite))(fd, buf, count, offset);
}

ssize_t read(int fd, void* buf, size_t count) {
    init_passthrough_if_needed();
    if (is_env_initialized && file_map.exist(fd)) {
        return pread(fd, buf, count, 0);
    }
    return (reinterpret_cast<decltype(&read)>(libc_read))(fd, buf, count);
}

ssize_t pread(int fd, void* buf, size_t count, off_t offset) {
    init_passthrough_if_needed();
    if (is_env_initialized && file_map.exist(fd)) {
        auto adafs_fd = file_map.get(fd);
        auto path = adafs_fd->path();
        size_t read_size = 0;
        int err;
        err = rpc_send_read(ipc_read_data_id, rpc_read_data_id, path, count, offset, buf, read_size);
        // TODO check how much we need to deal with the read_size
        return err == 0 ? read_size : 0;
    }
    return (reinterpret_cast<decltype(&pread)>(libc_pread))(fd, buf, count, offset);
}

ssize_t pread64(int fd, void* buf, size_t nbyte, __off64_t offset) {
    init_passthrough_if_needed();
    if (is_env_initialized && file_map.exist(fd)) {
//        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return size written
        return 0; // TODO
    }
    return (reinterpret_cast<decltype(&pread64)>(libc_pread64))(fd, buf, nbyte, offset);
}

off_t lseek(int fd, off_t offset, int whence) __THROW {
    init_passthrough_if_needed();
    return (reinterpret_cast<decltype(&lseek)>(libc_lseek))(fd, offset, whence);
}

off_t lseek64(int fd, off_t offset, int whence) __THROW {
    init_passthrough_if_needed();
    return lseek(fd, offset, whence);
}

int truncate(const char* path, off_t length) __THROW {
    init_passthrough_if_needed();
    return (reinterpret_cast<decltype(&truncate)>(libc_truncate))(path, length);
}

int ftruncate(int fd, off_t length) __THROW {
    init_passthrough_if_needed();
    return (reinterpret_cast<decltype(&ftruncate)>(libc_ftruncate))(fd, length);
}

int dup(int oldfd) __THROW {
    init_passthrough_if_needed();
    if (is_env_initialized && file_map.exist(oldfd)) {
        // Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&dup)>(libc_dup))(oldfd);
}

int dup2(int oldfd, int newfd) __THROW {
    init_passthrough_if_needed();
    if (is_env_initialized && (file_map.exist(oldfd) || file_map.exist(newfd))) {
        // Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&dup2)>(libc_dup2))(oldfd, newfd);
}

/**
 * Initializes the Argobots environment
 * @return
 */
bool init_ld_argobots() {
    ld_logger->info("Initializing Argobots ...");

    // We need no arguments to init
    auto argo_err = ABT_init(0, nullptr);
    if (argo_err != 0) {
        ld_logger->info("ABT_init() Failed to init Argobots (client)");
        return false;
    }
    // Set primary execution stream to idle without polling. Normally xstreams cannot sleep. This is what ABT_snoozer does
    argo_err = ABT_snoozer_xstream_self_set();
    if (argo_err != 0) {
        ld_logger->info("ABT_snoozer_xstream_self_set()  (client)");
        return false;
    }
    ld_logger->info("Success.");
    return true;
}

bool get_addr_by_hostid(const uint64_t hostid, hg_addr_t& svr_addr) {

    if (rpc_address_cache_.tryGet(hostid, svr_addr)) {
        ld_logger->trace("tryGet successful and put in svr_addr");
        //found
        return true;
    } else {
        ld_logger->trace("not found in lrucache");
        // not found, manual lookup and add address mapping to LRU cache
        auto hostname = RPC_PROTOCOL + "://"s + fs_config->hosts.at(hostid) + ":"s +
                        fs_config->rpc_port; // convert hostid to hostname and port
        ld_logger->trace("generated hostname {} with rpc_port {}", hostname, fs_config->rpc_port);
        margo_addr_lookup(margo_rpc_id_, hostname.c_str(), &svr_addr);
        if (svr_addr == HG_ADDR_NULL)
            return false;
        rpc_address_cache_.insert(hostid, svr_addr);
        return true;
    }
}

size_t get_rpc_node(const string& to_hash) {
    return std::hash<string>{}(to_hash) % fs_config->host_size;
}

bool is_local_op(const size_t recipient) {
    return recipient == fs_config->host_id;
}

void register_client_ipcs(margo_instance_id mid) {
    minimal_id = MARGO_REGISTER(mid, "rpc_minimal", rpc_minimal_in_t, rpc_minimal_out_t, NULL);
    ipc_open_id = MARGO_REGISTER(mid, "rpc_srv_open", rpc_open_in_t, rpc_err_out_t, NULL);
    ipc_stat_id = MARGO_REGISTER(mid, "rpc_srv_stat", rpc_stat_in_t, rpc_stat_out_t, NULL);
    ipc_unlink_id = MARGO_REGISTER(mid, "ipc_srv_unlink", ipc_unlink_in_t, ipc_err_out_t, NULL);
    ipc_update_metadentry_id = MARGO_REGISTER(mid, "rpc_srv_update_metadentry", rpc_update_metadentry_in_t,
                                              rpc_err_out_t, NULL);
    ipc_update_metadentry_size_id = MARGO_REGISTER(mid, "rpc_srv_update_metadentry_size",
                                                   rpc_update_metadentry_size_in_t, rpc_update_metadentry_size_out_t,
                                                   NULL);
    ipc_config_id = MARGO_REGISTER(mid, "ipc_srv_fs_config", ipc_config_in_t, ipc_config_out_t,
                                   NULL);
    ipc_write_data_id = MARGO_REGISTER(mid, "rpc_srv_write_data", rpc_write_data_in_t, rpc_data_out_t,
                                       NULL);
    ipc_read_data_id = MARGO_REGISTER(mid, "rpc_srv_read_data", rpc_read_data_in_t, rpc_data_out_t,
                                      NULL);
}

void register_client_rpcs(margo_instance_id mid) {
    rpc_minimal_id = MARGO_REGISTER(mid, "rpc_minimal", rpc_minimal_in_t, rpc_minimal_out_t, NULL);
    rpc_open_id = MARGO_REGISTER(mid, "rpc_srv_open", rpc_open_in_t, rpc_err_out_t, NULL);
    rpc_stat_id = MARGO_REGISTER(mid, "rpc_srv_stat", rpc_stat_in_t, rpc_stat_out_t, NULL);
    rpc_remove_node_id = MARGO_REGISTER(mid, "rpc_srv_remove_node", rpc_remove_node_in_t,
                                        rpc_err_out_t, NULL);
    rpc_update_metadentry_id = MARGO_REGISTER(mid, "rpc_srv_update_metadentry", rpc_update_metadentry_in_t,
                                              rpc_err_out_t, NULL);
    rpc_update_metadentry_size_id = MARGO_REGISTER(mid, "rpc_srv_update_metadentry_size",
                                                   rpc_update_metadentry_size_in_t, rpc_update_metadentry_size_out_t,
                                                   NULL);
    rpc_write_data_id = MARGO_REGISTER(mid, "rpc_srv_write_data", rpc_write_data_in_t, rpc_data_out_t,
                                       NULL);
    rpc_read_data_id = MARGO_REGISTER(mid, "rpc_srv_read_data", rpc_read_data_in_t, rpc_data_out_t,
                                      NULL);
}

bool init_margo_client(Margo_mode mode, const string na_plugin) {

    ABT_xstream xstream = ABT_XSTREAM_NULL;
    ABT_pool pool = ABT_POOL_NULL;

    // get execution stream and its main pools
    auto ret = ABT_xstream_self(&xstream);
    if (ret != ABT_SUCCESS)
        return false;
    ret = ABT_xstream_get_main_pools(xstream, 1, &pool);
    if (ret != ABT_SUCCESS) return false;
    if (mode == Margo_mode::IPC)
        ld_logger->info("Initializing Mercury IPC client ...");
    else
        ld_logger->info("Initializing Mercury RPC client ...");
    /* MERCURY PART */
    // Init Mercury layer (must be finalized when finished)
    hg_class_t* hg_class;
    hg_context_t* hg_context;
    hg_class = HG_Init(na_plugin.c_str(), HG_FALSE);
    if (hg_class == nullptr) {
        ld_logger->info("HG_Init() Failed to init Mercury client layer");
        return false;
    }
    // Create a new Mercury context (must be destroyed when finished)
    hg_context = HG_Context_create(hg_class);
    if (hg_context == nullptr) {
        ld_logger->info("HG_Context_create() Failed to create Mercury client context");
        HG_Finalize(hg_class);
        return false;
    }
    ld_logger->info("Success.");

    /* MARGO PART */
    if (mode == Margo_mode::IPC)
        ld_logger->info("Initializing Margo IPC client ...");
    else
        ld_logger->info("Initializing Margo RPC client ...");
    // margo will run in the context of thread
    auto mid = margo_init_pool(pool, pool, hg_context);
    if (mid == MARGO_INSTANCE_NULL) {
        ld_logger->error("margo_init_pool failed to initialize the Margo client");
        return false;
    }
    ld_logger->info("Success.");

    if (mode == Margo_mode::IPC) {
        margo_ipc_id_ = mid;
        auto adafs_daemon_pid = getProcIdByName("adafs_daemon"s);
        if (adafs_daemon_pid == -1) {
            ld_logger->error("{}() ADA-FS daemon not started. Exiting ...", __func__);
            return false;
        }
        ld_logger->info("{}() ADA-FS daemon with PID {} found.", __func__, adafs_daemon_pid);

        string sm_addr_str = "na+sm://"s + to_string(adafs_daemon_pid) + "/0";
        margo_addr_lookup(margo_ipc_id_, sm_addr_str.c_str(), &daemon_svr_addr_);

        register_client_ipcs(mid);
    } else {
        margo_rpc_id_ = mid;
        register_client_rpcs(mid);
    }
//    margo_diag_start(mid);

//    for (int i = 0; i < 10; ++i) {
//        printf("Running %d iteration\n", i);
//        send_minimal_ipc(minimal_id);
//    }

    return true;
}

margo_instance_id ld_margo_ipc_id() {
    return margo_ipc_id_;
}

margo_instance_id ld_margo_rpc_id() {
    return margo_rpc_id_;
}

hg_addr_t daemon_addr() {
    return daemon_svr_addr_;
}

/**
 * This function is only called in the preload constructor
 */
void init_environment() {
    // init margo client for IPC
    auto err = init_ld_argobots();
    assert(err);
    err = init_margo_client(Margo_mode::IPC, "na+sm"s);
    assert(err);
    err = ipc_send_get_fs_config(ipc_config_id); // get fs configurations the daemon was started with.
    assert(err);
    err = init_margo_client(Margo_mode::RPC, RPC_PROTOCOL);
    assert(err);
    is_env_initialized = true;
    ld_logger->info("Environment initialized.");
}

void init_passthrough_() {
    libc = dlopen("libc.so.6", RTLD_LAZY);
    libc_open = dlsym(libc, "open");
//    libc_fopen = dlsym(libc, "fopen");
//    libc_fopen64 = dlsym(libc, "fopen64");

    libc_unlink = dlsym(libc, "unlink");

    libc_close = dlsym(libc, "close");
//    libc___close = dlsym(libc, "__close");

    libc_stat = dlsym(libc, "stat");
    libc_fstat = dlsym(libc, "fstat");
    libc___xstat = dlsym(libc, "__xstat");
    libc___xstat64 = dlsym(libc, "__xstat64");
    libc___fxstat = dlsym(libc, "__fxstat");
    libc___fxstat64 = dlsym(libc, "__fxstat64");
    libc___lxstat = dlsym(libc, "__lxstat");
    libc___lxstat64 = dlsym(libc, "__lxstat64");

    libc_access = dlsym(libc, "access");

    libc_puts = dlsym(libc, "puts");

    libc_write = dlsym(libc, "write");
    libc_pwrite = dlsym(libc, "pwrite");
    libc_read = dlsym(libc, "read");
    libc_pread = dlsym(libc, "pread");
    libc_pread64 = dlsym(libc, "pread64");

    libc_lseek = dlsym(libc, "lseek");

    libc_truncate = dlsym(libc, "truncate");
    libc_ftruncate = dlsym(libc, "ftruncate");

    libc_dup = dlsym(libc, "dup");
    libc_dup2 = dlsym(libc, "dup2");

    fs_config = make_shared<struct FsConfig>();
    //set the spdlogger and initialize it with spdlog
    ld_logger = spdlog::basic_logger_mt("basic_logger", LOG_PRELOAD_PATH);
    // set logger format
    spdlog::set_pattern("[%C-%m-%d %H:%M:%S.%f] %P [%L] %v");
    // flush log when info, warning, error messages are encountered
    ld_logger->flush_on(spdlog::level::info);
#if defined(LOG_PRELOAD_TRACE)
    spdlog::set_level(spdlog::level::trace);
    ld_logger->flush_on(spdlog::level::trace);
#elif defined(LOG_PRELOAD_DEBUG)
    spdlog::set_level(spdlog::level::debug);
//    ld_logger->flush_on(spdlog::level::debug);
#elif defined(LOG_PRELOAD_INFO)
    spdlog::set_level(spdlog::level::info);
//    ld_logger->flush_on(spdlog::level::info);
#else
    spdlog::set_level(spdlog::level::off);
#endif

    ld_logger->info("Passthrough initialized.");
}

void init_passthrough_if_needed() {
    pthread_once(&init_lib_thread, init_passthrough_);
}

/**
 * Called initially when preload library is used with the LD_PRELOAD environment variable
 */
void init_preload() {
    init_passthrough_if_needed();
    init_environment();
    ld_logger->info("{}() successful.", __func__);
}

/**
 * Called last when preload library is used with the LD_PRELOAD environment variable
 */
void destroy_preload() {
//    margo_diag_dump(margo_ipc_id_, "-", 0);
    ld_logger->info("Freeing Mercury daemon addr ...");
    HG_Addr_free(margo_get_class(margo_ipc_id_), daemon_svr_addr_);
    ld_logger->info("Finalizing Margo IPC client ...");
    auto mercury_ipc_class = margo_get_class(margo_ipc_id_);
    auto mercury_ipc_context = margo_get_context(margo_ipc_id_);
    margo_finalize(margo_ipc_id_);

    ld_logger->info("Freeing Mercury RPC addresses ...");
    // free all rpc addresses in LRU map and finalize margo rpc
    auto free_all_addr = [&](const KVCache::node_type& n) {
        HG_Addr_free(margo_get_class(ld_margo_rpc_id()), n.value);
    };
    rpc_address_cache_.cwalk(free_all_addr);
    ld_logger->info("Finalizing Margo RPC client ...");
    auto mercury_rpc_class = margo_get_class(margo_rpc_id_);
    auto mercury_rpc_context = margo_get_context(margo_rpc_id_);
    margo_finalize(margo_rpc_id_);

    ld_logger->info("Destroying Mercury context ...");
    HG_Context_destroy(mercury_ipc_context);
    HG_Context_destroy(mercury_rpc_context);
    ld_logger->info("Finalizing Mercury class ...");
    HG_Finalize(mercury_ipc_class);
    HG_Finalize(mercury_rpc_class);
    ld_logger->info("Preload library shut down.");

    ld_logger->info("Finalizing Argobots ...");
    ABT_finalize();
}