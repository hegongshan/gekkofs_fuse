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
bool is_lib_initialized = false;

// external variables
FILE* debug_fd;
shared_ptr<FsConfig> fs_config;

// function pointer for preloading
void* libc;

void* libc_open;
//void* libc_open64; //unused
void* libc_fopen;

//void* libc_creat; //unused
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

int ld_open(const char* path, int flags, ...) {
    DAEMON_DEBUG(debug_fd, "ld_open called with path %s\n", path);
    mode_t mode;
    if (flags & O_CREAT) {
        va_list vl;
        va_start(vl, flags);
        mode = va_arg(vl, int);
        va_end(vl);
    }
    if (is_fs_path(path)) {
        auto fd = file_map.add(path, (flags & O_APPEND) != 0);
        // TODO call daemon and return if successful return the above fd. if unsuccessful delete fd remove file from map
#ifndef MARGOIPC

#else
        auto err = ipc_send_open(path, flags, mode, ipc_open_id);
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

int ld_open64(__const char* path, int flags, ...) {
    DAEMON_DEBUG(debug_fd, "ld_open64 called with path %s\n", path);
    mode_t mode;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return ld_open(path, flags | O_LARGEFILE, mode);
}

FILE* ld_fopen(const char* path, const char* mode) {
//    DAEMON_DEBUG(debug_fd, "ld_fopen called with path %s with mode %d\n", path, mode);
    return (reinterpret_cast<decltype(&fopen)>(libc_fopen))(path, mode);
}

int ld_creat(const char* path, mode_t mode) {
    DAEMON_DEBUG(debug_fd, "ld_creat called with path %s with mode %d\n", path, mode);
    return ld_open(path, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

int ld_unlink(const char* path) __THROW {
//    DAEMON_DEBUG(debug_fd, "ld_unlink called with path %s\n", path);
    if (is_fs_path(path)) {
#ifndef MARGOIPC
#else
        return ipc_send_unlink(path, ipc_unlink_id);
#endif
    }
    return (reinterpret_cast<decltype(&unlink)>(libc_unlink))(path);
}

int ld_close(int fd) {
    if (file_map.exist(fd)) {
        // Currently no call to the daemon is required
        file_map.remove(fd);
        return 0;
    }
    return (reinterpret_cast<decltype(&close)>(libc_close))(fd);
}

int ld___close(int fd) {
    return ld_close(fd);
}


int ld_stat(const char* path, struct stat* buf) __THROW {
    DAEMON_DEBUG(debug_fd, "stat called with path %s\n", path);
    if (is_fs_path(path)) {
        // TODO call daemon and return
#ifndef MARGOIPC

#else
        return ipc_send_stat(path, buf, ipc_stat_id);
#endif
        return 0; // TODO
    }
    return (reinterpret_cast<decltype(&stat)>(libc_stat))(path, buf);
}

int ld_fstat(int fd, struct stat* buf) __THROW {
    DAEMON_DEBUG(debug_fd, "ld_fstat called with fd %d\n", fd);
    if (file_map.exist(fd)) {
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

int ld___xstat(int ver, const char* path, struct stat* buf) __THROW {
    DAEMON_DEBUG(debug_fd, "ld___xstat called with path %s\n", path);
    if (is_fs_path(path)) {
        // TODO call stat
#ifndef MARGOIPC

#else
        return ipc_send_stat(path, buf, ipc_stat_id);
#endif
    }
    return (reinterpret_cast<decltype(&__xstat)>(libc___xstat))(ver, path, buf);
}

int ld___xstat64(int ver, const char* path, struct stat64* buf) __THROW {
    DAEMON_DEBUG(debug_fd, "ld___xstat64 called with path %s\n", path);
//    if (is_fs_path(path)) {
//        // Not implemented
//        return -1;
//    }
    return (reinterpret_cast<decltype(&__xstat64)>(libc___xstat64))(ver, path, buf);
}

int ld___fxstat(int ver, int fd, struct stat* buf) __THROW {
    DAEMON_DEBUG(debug_fd, "ld___fxstat called with fd %d\n", fd);
    if (file_map.exist(fd)) {
        // TODO call fstat
        auto path = file_map.get(fd)->path();
#ifndef MARGOIPC

#else
        return ipc_send_stat(path, buf, ipc_stat_id);
#endif
    }
    return (reinterpret_cast<decltype(&__fxstat)>(libc___fxstat))(ver, fd, buf);
}

int ld___fxstat64(int ver, int fd, struct stat64* buf) __THROW {
    DAEMON_DEBUG(debug_fd, "ld___fxstat64 called with fd %s\n", fd);
    if (file_map.exist(fd)) {
        // TODO call fstat64
        auto path = file_map.get(fd)->path();
#ifndef MARGOIPC

#else
//        return ipc_send_stat(path, buf, ipc_stat_id); // TODO need new function for stat64 struct
#endif
    }
    return (reinterpret_cast<decltype(&__fxstat64)>(libc___fxstat64))(ver, fd, buf);
}

extern int ld___lxstat(int ver, const char* path, struct stat* buf) __THROW {
    if (is_fs_path(path)) {
        // Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&__lxstat)>(libc___lxstat))(ver, path, buf);
}

extern int ld___lxstat64(int ver, const char* path, struct stat64* buf) __THROW {
    if (is_fs_path(path)) {
        // Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&__lxstat64)>(libc___lxstat64))(ver, path, buf);
}

int ld_access(const char* path, int mode) __THROW {
    if (is_fs_path(path)) {
#ifndef MARGOIPC

#else

#endif
    }
    return (reinterpret_cast<decltype(&access)>(libc_access))(path, mode);
}

int ld_puts(const char* str) {
    return (reinterpret_cast<decltype(&puts)>(libc_puts))(str);
}

ssize_t ld_write(int fd, const void* buf, size_t count) {
    if (file_map.exist(fd)) {
        auto adafs_fd = file_map.get(fd);
        auto path = adafs_fd->path(); // TODO use this to send to the daemon (call directly)
        auto append_flag = adafs_fd->append_flag();
        // TODO call daemon and return size written
#ifndef MARGOIPC

#else

#endif
        return 0; // TODO
    }
    return (reinterpret_cast<decltype(&write)>(libc_write))(fd, buf, count);
}

ssize_t ld_pwrite(int fd, const void* buf, size_t count, off_t offset) {
    if (file_map.exist(fd)) {
        auto adafs_fd = file_map.get(fd);
        auto path = adafs_fd->path(); // TODO use this to send to the daemon (call directly)
        auto append_flag = adafs_fd->append_flag();
        // TODO call daemon and return size written
#ifndef MARGOIPC

#else

#endif
        return 0;
    }
    return (reinterpret_cast<decltype(&pwrite)>(libc_pwrite))(fd, buf, count, offset);
}

ssize_t ld_read(int fd, void* buf, size_t count) {
    if (file_map.exist(fd)) {
        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return size read
#ifndef MARGOIPC

#else

#endif
        return 0; // TODO
    }
    return (reinterpret_cast<decltype(&read)>(libc_read))(fd, buf, count);
}

ssize_t ld_pread(int fd, void* buf, size_t count, off_t offset) {
    if (file_map.exist(fd)) {
        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return size written
#ifndef MARGOIPC

#else

#endif
        return 0; // TODO
    }
    return (reinterpret_cast<decltype(&pread)>(libc_pread))(fd, buf, count, offset);
}

ssize_t ld_pread64(int fd, void* buf, size_t nbyte, __off64_t offset) {
    if (file_map.exist(fd)) {
        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return size written
#ifndef MARGOIPC

#else

#endif
        return 0; // TODO
    }
    return (reinterpret_cast<decltype(&pread64)>(libc_pread64))(fd, buf, nbyte, offset);
}

off_t ld_lseek(int fd, off_t offset, int whence) __THROW {
    return (reinterpret_cast<decltype(&lseek)>(libc_lseek))(fd, offset, whence);
}

off_t ld_lseek64(int fd, off_t offset, int whence) __THROW {
    return ld_lseek(fd, offset, whence);
}

int ld_truncate(const char* path, off_t length) __THROW {
    return (reinterpret_cast<decltype(&truncate)>(libc_truncate))(path, length);
}

int ld_ftruncate(int fd, off_t length) __THROW {
    return (reinterpret_cast<decltype(&ftruncate)>(libc_ftruncate))(fd, length);
}

int ld_dup(int oldfd) __THROW {
    if (file_map.exist(oldfd)) {
        // Not implemented
        return -1;
    }
    return (reinterpret_cast<decltype(&dup)>(libc_dup))(oldfd);
}

int ld_dup2(int oldfd, int newfd) __THROW {
    if (file_map.exist(oldfd) || file_map.exist(newfd)) {
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
    auto protocol_port = "bmi+tcp"s;
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

    margo_addr_lookup(margo_id_, "bmi+tcp://localhost:4433", &daemon_svr_addr_);

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
 * Called initially when preload library is used with the LD_PRELOAD environment variable
 */
void init_preload(void) {

    // just a security measure
    if (is_lib_initialized)
        return;
    libc = dlopen("libc.so.6", RTLD_LAZY);
    libc_open = dlsym(libc, "open");
    libc_fopen = dlsym(libc, "fopen");

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

    // XXX Continue here. stat is not intercepted because it is not part of libc? investigate.

    debug_fd = fopen(LOG_DAEMON_PATH, "a+");
    fs_config = make_shared<struct FsConfig>();
    DAEMON_DEBUG0(debug_fd, "Preload initialized.\n");
    is_lib_initialized = true;
#ifdef MARGOIPC
    // init margo client for IPC
    init_ld_argobots();
    init_ipc_client();
    ipc_send_get_fs_config(ipc_config_id); // get fs configurations the daemon was started with.
#endif
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