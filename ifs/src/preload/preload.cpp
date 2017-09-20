//
// Created by evie on 7/21/17.
//

#include <preload/preload.hpp>
#include <preload/ipc_types.hpp>
#include <preload/margo_ipc.hpp>

#include <dlfcn.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../../main.hpp"

static pthread_once_t init_lib_thread = PTHREAD_ONCE_INIT;

// Mercury Client
hg_class_t* mercury_hg_class_;
hg_context_t* mercury_hg_context_;
margo_instance_id margo_id_;
hg_addr_t daemon_svr_addr_ = HG_ADDR_NULL;

static hg_id_t minimal_id;
static hg_id_t ipc_config_id;
static hg_id_t ipc_open_id;
static hg_id_t ipc_stat_id;
static hg_id_t ipc_unlink_id;

// misc
static std::atomic<bool> is_env_initialized(false);

// external variables
FILE* debug_fd;
shared_ptr<FsConfig> fs_config;

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
    DAEMON_DEBUG(debug_fd, "open called with path %s\n", path);
    mode_t mode;
    if (flags & O_CREAT) {
        va_list vl;
        va_start(vl, flags);
        mode = va_arg(vl, int);
        va_end(vl);
    }
    if (is_env_initialized && is_fs_path(path)) {
        auto fd = file_map.add(path, (flags & O_APPEND) != 0);
#ifndef MARGOIPC

#else
        auto err = ipc_send_open(path, flags, mode, ipc_open_id);
//        auto err = 0;
#endif
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
    DAEMON_DEBUG(debug_fd, "open64 called with path %s\n", path);
    mode_t mode;
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
    DAEMON_DEBUG(debug_fd, "creat called with path %s with mode %d\n", path, mode);
    return open(path, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

#undef creat64

int creat64(const char* path, mode_t mode) {
    init_passthrough_if_needed();
    DAEMON_DEBUG(debug_fd, "creat64 called with path %s with mode %d\n", path, mode);
    return open(path, O_CREAT | O_WRONLY | O_TRUNC | O_LARGEFILE, mode);
}

int unlink(const char* path) __THROW {
    init_passthrough_if_needed();
//    DAEMON_DEBUG(debug_fd, "unlink called with path %s\n", path);
    if (is_env_initialized && is_fs_path(path)) {
#ifndef MARGOIPC
#else
        return ipc_send_unlink(path, ipc_unlink_id);
#endif
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


int stat(const char* path, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    DAEMON_DEBUG(debug_fd, "stat called with path %s\n", path);
    if (is_env_initialized && is_fs_path(path)) {
        // TODO call daemon and return
#ifndef MARGOIPC

#else
        return ipc_send_stat(path, buf, ipc_stat_id);
#endif
        return 0; // TODO
    }
    return (reinterpret_cast<decltype(&stat)>(libc_stat))(path, buf);
}

int fstat(int fd, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    DAEMON_DEBUG(debug_fd, "fstat called with fd %d\n", fd);
    if (is_env_initialized && file_map.exist(fd)) {
        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return
#ifndef MARGOIPC

#else
        return ipc_send_stat(path, buf, ipc_stat_id);
#endif
        return 0; // TODO
    }
    return (reinterpret_cast<decltype(&fstat)>(libc_fstat))(fd, buf);
}

int __xstat(int ver, const char* path, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    DAEMON_DEBUG(debug_fd, "__xstat called with path %s\n", path);
    if (is_env_initialized && is_fs_path(path)) {
        // TODO call stat
#ifndef MARGOIPC

#else
        return ipc_send_stat(path, buf, ipc_stat_id);
#endif
    }
    return (reinterpret_cast<decltype(&__xstat)>(libc___xstat))(ver, path, buf);
}

int __xstat64(int ver, const char* path, struct stat64* buf) __THROW {
    init_passthrough_if_needed();
    DAEMON_DEBUG(debug_fd, "__xstat64 called with path %s\n", path);
    if (is_env_initialized && is_fs_path(path)) {
        // Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&__xstat64)>(libc___xstat64))(ver, path, buf);
}

int __fxstat(int ver, int fd, struct stat* buf) __THROW {
    init_passthrough_if_needed();
    DAEMON_DEBUG(debug_fd, "__fxstat called with fd %d\n", fd);
    if (is_env_initialized && file_map.exist(fd)) {
        // TODO call fstat
        auto path = file_map.get(fd)->path();
#ifndef MARGOIPC

#else
        return ipc_send_stat(path, buf, ipc_stat_id);
#endif
    }
    return (reinterpret_cast<decltype(&__fxstat)>(libc___fxstat))(ver, fd, buf);
}

int __fxstat64(int ver, int fd, struct stat64* buf) __THROW {
    init_passthrough_if_needed();
    DAEMON_DEBUG(debug_fd, "__fxstat64 called with fd %d\n", fd);
    if (is_env_initialized && file_map.exist(fd)) {
        // TODO call fstat64
//        auto path = file_map.get(fd)->path();
#ifndef MARGOIPC

#else
//        return ipc_send_stat(path, buf, ipc_stat_id); // TODO need new function for stat64 struct
#endif
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
#ifndef MARGOIPC

#else

#endif
    }
    return (reinterpret_cast<decltype(&access)>(libc_access))(path, mode);
}

int puts(const char* str) {
    return (reinterpret_cast<decltype(&puts)>(libc_puts))(str);
}

ssize_t write(int fd, const void* buf, size_t count) {
    init_passthrough_if_needed();
    if (is_env_initialized && file_map.exist(fd)) {
//        auto adafs_fd = file_map.get(fd);
//        auto path = adafs_fd->path(); // TODO use this to send to the daemon (call directly)
//        auto append_flag = adafs_fd->append_flag();
        // TODO call daemon and return size written
#ifndef MARGOIPC

#else

#endif
        return 0; // TODO
    }
    return (reinterpret_cast<decltype(&write)>(libc_write))(fd, buf, count);
}

ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset) {
    init_passthrough_if_needed();
    if (is_env_initialized && file_map.exist(fd)) {
//        auto adafs_fd = file_map.get(fd);
//        auto path = adafs_fd->path(); // TODO use this to send to the daemon (call directly)
//        auto append_flag = adafs_fd->append_flag();
        // TODO call daemon and return size written
#ifndef MARGOIPC

#else

#endif
        return 0;
    }
    return (reinterpret_cast<decltype(&pwrite)>(libc_pwrite))(fd, buf, count, offset);
}

ssize_t read(int fd, void* buf, size_t count) {
    init_passthrough_if_needed();
    if (is_env_initialized && file_map.exist(fd)) {
//        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return size read
#ifndef MARGOIPC

#else

#endif
        return 0; // TODO
    }
    return (reinterpret_cast<decltype(&read)>(libc_read))(fd, buf, count);
}

ssize_t pread(int fd, void* buf, size_t count, off_t offset) {
    init_passthrough_if_needed();
    if (is_env_initialized && file_map.exist(fd)) {
//        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return size written
#ifndef MARGOIPC

#else

#endif
        return 0; // TODO
    }
    return (reinterpret_cast<decltype(&pread)>(libc_pread))(fd, buf, count, offset);
}

ssize_t pread64(int fd, void* buf, size_t nbyte, __off64_t offset) {
    init_passthrough_if_needed();
    if (is_env_initialized && file_map.exist(fd)) {
//        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return size written
#ifndef MARGOIPC

#else

#endif
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
    DAEMON_DEBUG0(debug_fd, "Initializing Argobots ...\n");

    // We need no arguments to init
    auto argo_err = ABT_init(0, nullptr);
    if (argo_err != 0) {
        DAEMON_DEBUG0(debug_fd, "ABT_init() Failed to init Argobots (client)\n");
        return false;
    }
    // Set primary execution stream to idle without polling. Normally xstreams cannot sleep. This is what ABT_snoozer does
    argo_err = ABT_snoozer_xstream_self_set();
    if (argo_err != 0) {
        DAEMON_DEBUG0(debug_fd, "ABT_snoozer_xstream_self_set()  (client)\n");
        return false;
    }
    DAEMON_DEBUG0(debug_fd, "Success.\n");
    return true;
}

void register_client_ipcs() {
    minimal_id = MERCURY_REGISTER(mercury_hg_class_, "rpc_minimal", rpc_minimal_in_tt, rpc_minimal_out_tt, nullptr);
    ipc_open_id = MERCURY_REGISTER(mercury_hg_class_, "ipc_srv_open", ipc_open_in_t, ipc_res_out_t, nullptr);
    ipc_stat_id = MERCURY_REGISTER(mercury_hg_class_, "ipc_srv_stat", ipc_stat_in_t, ipc_stat_out_t, nullptr);
    ipc_unlink_id = MERCURY_REGISTER(mercury_hg_class_, "ipc_srv_unlink", ipc_unlink_in_t, ipc_res_out_t, nullptr);
    ipc_config_id = MERCURY_REGISTER(mercury_hg_class_, "ipc_srv_fs_config", ipc_config_in_t, ipc_config_out_t,
                                     nullptr);
}

bool init_ipc_client() {
    auto protocol_port = "na+sm"s;
    DAEMON_DEBUG0(debug_fd, "Initializing Mercury client ...\n");
    /* MERCURY PART */
    // Init Mercury layer (must be finalized when finished)
    hg_class_t* hg_class;
    hg_context_t* hg_context;
    hg_class = HG_Init(protocol_port.c_str(), HG_FALSE);
    if (hg_class == nullptr) {
        DAEMON_DEBUG0(debug_fd, "HG_Init() Failed to init Mercury client layer\n");
        return false;
    }
    // Create a new Mercury context (must be destroyed when finished)
    hg_context = HG_Context_create(hg_class);
    if (hg_context == nullptr) {
        DAEMON_DEBUG0(debug_fd, "HG_Context_create() Failed to create Mercury client context\n");
        HG_Finalize(hg_class);
        return false;
    }
    DAEMON_DEBUG0(debug_fd, "Success.\n");

    /* MARGO PART */
    DAEMON_DEBUG0(debug_fd, "Initializing Margo client ...\n");
    // Start Margo
    auto mid = margo_init(0, 0,
                          hg_context);
    if (mid == MARGO_INSTANCE_NULL) {
        DAEMON_DEBUG0(debug_fd, "[ERR]: margo_init failed to initialize the Margo client\n");
        HG_Context_destroy(hg_context);
        HG_Finalize(hg_class);
        return false;
    }
    DAEMON_DEBUG0(debug_fd, "Success.\n");

    // Put context and class into RPC_data object
    mercury_hg_class_ = hg_class;
    mercury_hg_context_ = hg_context;
    margo_id_ = mid;

    auto adafs_daemon_pid = getProcIdByName("adafs_daemon"s);
    if (adafs_daemon_pid == -1) {
        printf("[ERR] ADA-FS daemon not started. Exiting ...\n");
        return false;
    }
    printf("[INFO] ADA-FS daemon with PID %d found.\n", adafs_daemon_pid);

    string sm_addr_str = "na+sm://"s + fmt::FormatInt(adafs_daemon_pid).str() + "/0";
    margo_addr_lookup(margo_id_, sm_addr_str.c_str(), &daemon_svr_addr_);

    register_client_ipcs();

//    for (int i = 0; i < 10000; ++i) {
//        printf("Running %d iteration\n", i);
//        send_minimal_rpc(minimal_id);
//    }

    return true;
}

hg_class_t* ld_mercury_class() {
    return mercury_hg_class_;
}

hg_context_t* ld_mercury_context() {
    return mercury_hg_context_;
}

margo_instance_id ld_margo_id() {
    return margo_id_;
}

hg_addr_t daemon_addr() {
    return daemon_svr_addr_;
}

/**
 * This function is only called in the preload constructor
 */
void init_environment() {
#ifdef MARGOIPC
    // init margo client for IPC
    auto err = init_ld_argobots();
    assert(err);
    err = init_ipc_client();
    assert(err);
    err = ipc_send_get_fs_config(ipc_config_id); // get fs configurations the daemon was started with.
    assert(err);
#endif
    is_env_initialized = true;
    DAEMON_DEBUG0(debug_fd, "Environment initialized.\n");
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

    debug_fd = fopen(LOG_DAEMON_PATH, "a+");
    fs_config = make_shared<struct FsConfig>();
    DAEMON_DEBUG0(debug_fd, "Passthrough initialized.\n");
}

void init_passthrough_if_needed() {
    pthread_once(&init_lib_thread, init_passthrough_);
}

/**
 * Called initially when preload library is used with the LD_PRELOAD environment variable
 */
void init_preload(void) {
    init_passthrough_if_needed();
    init_environment();
    printf("[INFO] preload init successful.\n");
}

/**
 * Called last when preload library is used with the LD_PRELOAD environment variable
 */
void destroy_preload(void) {

#ifdef MARGOIPC
    DAEMON_DEBUG0(debug_fd, "Freeing Mercury daemon addr ...\n");
    HG_Addr_free(mercury_hg_class_, daemon_svr_addr_);
    DAEMON_DEBUG0(debug_fd, "Finalizing Margo ...\n");
    margo_finalize(margo_id_);

    DAEMON_DEBUG0(debug_fd, "Finalizing Argobots ...\n");
    ABT_finalize();

    DAEMON_DEBUG0(debug_fd, "Destroying Mercury context ...\n");
    HG_Context_destroy(mercury_hg_context_);
    DAEMON_DEBUG0(debug_fd, "Finalizing Mercury class ...\n");
    HG_Finalize(mercury_hg_class_);
    DAEMON_DEBUG0(debug_fd, "Preload library shut down.\n");
#endif
    fclose(debug_fd);
}