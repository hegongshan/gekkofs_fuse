//
// Created by evie on 7/21/17.
//

//#define _GNU_SOURCE
#include <preload/preload.hpp>
#include <preload/ipc_types.hpp>

#include <dlfcn.h>
#include <stdarg.h>
#include <unistd.h>

// Mercury Client
hg_class_t* mercury_hg_class;
hg_context_t* mercury_hg_context;
margo_instance_id margo_mid;
hg_addr_t daemon_addr;

hg_id_t ipc_open_id;

int ld_open(const char* path, int flags, ...) {
    mode_t mode;
    if (flags & O_CREAT) {
        va_list vl;
        va_start(vl, flags);
        mode = va_arg(vl, int);
        va_end(vl);
    }
    if (is_fs_path(path)) {
        auto fd = file_map.add(path);
        // TODO call daemon and return if successful return the above fd. if unsuccessful delete fd remove file from map
#ifndef MARGOIPC

#else

#endif
    }
    return (reinterpret_cast<decltype(&open)>(libc_open))(path, flags, mode);
}

int ld_open64(__const char* path, int flags, ...) {
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
    printf("test\n");
    return (reinterpret_cast<decltype(&fopen)>(libc_fopen))(path, mode);
}

int ld_creat(const char* path, mode_t mode) {
    return ld_open(path, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

int ld_close(int fd) {
    if (file_map.exist(fd)) {
        // TODO call daemon and return (do we even need to)
#ifndef MARGOIPC

#else

#endif
        file_map.remove(fd);
    }
    return (reinterpret_cast<decltype(&close)>(libc_close))(fd);
}

int ld___close(int fd) {
    return ld_close(fd);
}


int ld_stat(const char* path, struct stat* buf) {
    if (is_fs_path(path)) {
        // TODO call daemon and return
#ifndef MARGOIPC

#else

#endif
    }
    return (reinterpret_cast<decltype(&stat)>(libc_stat))(path, buf);
}

int ld_fstat(int fd, struct stat* buf) {
    if (file_map.exist(fd)) {
        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return
#ifndef MARGOIPC

#else

#endif
    }
    return (reinterpret_cast<decltype(&fstat)>(libc_fstat))(fd, buf);
}

int ld_puts(const char* str) {
    return (reinterpret_cast<decltype(&puts)>(libc_puts))(str);
}

ssize_t ld_write(int fd, const void* buf, size_t count) {
    if (file_map.exist(fd)) {
        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return size written
#ifndef MARGOIPC

#else

#endif
    }
    return (reinterpret_cast<decltype(&write)>(libc_write))(fd, buf, count);
}

ssize_t ld_read(int fd, void* buf, size_t count) {
    if (file_map.exist(fd)) {
        auto path = file_map.get(fd)->path(); // TODO use this to send to the daemon (call directly)
        // TODO call daemon and return size written
#ifndef MARGOIPC

#else

#endif
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
bool init_argobots() {
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
    ipc_open_id = MERCURY_REGISTER(mercury_hg_class, "ipc_open", ipc_open_in_t, ipc_res_out_t, nullptr);
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
    mercury_hg_class = hg_class;
    mercury_hg_context = hg_context;
    margo_mid = mid;

    margo_addr_lookup(margo_mid, "bmi+tcp://localhost:4433", &daemon_addr);

    register_client_ipcs();

    return true;
}

void init_preload(void) {

    // just a security measure
    if (is_lib_initialized)
        return;
    libc = dlopen("libc.so.6", RTLD_LAZY);
    libc_open = dlsym(libc, "open");
    libc_fopen = dlsym(libc, "fopen");

    libc_close = dlsym(libc, "close");
//    libc___close = dlsym(libc, "__close");

    libc_stat = dlsym(libc, "stat");
    libc_fstat = dlsym(libc, "fstat");

    libc_puts = dlsym(libc, "puts");

    libc_write = dlsym(libc, "write");
    libc_read = dlsym(libc, "read");
    libc_pread = dlsym(libc, "pread");
    libc_pread64 = dlsym(libc, "pread64");

    libc_lseek = dlsym(libc, "lseek");

    libc_truncate = dlsym(libc, "truncate");
    libc_ftruncate = dlsym(libc, "ftruncate");

    libc_dup = dlsym(libc, "dup");
    libc_dup2 = dlsym(libc, "dup2");

    debug_fd = fopen(LOG_DAEMON_PATH, "a+");
    DAEMON_DEBUG0(debug_fd, "Preload initialized.\n");
    is_lib_initialized = true;
#ifdef MARGOIPC
    // init margo client for IPC
    init_argobots();
    init_ipc_client();
#endif
}

void destroy_preload(void) {

#ifdef MARGOIPC
    DAEMON_DEBUG0(debug_fd, "Freeing Mercury daemon addr ...\n");
    HG_Addr_free(mercury_hg_class, daemon_addr);
    DAEMON_DEBUG0(debug_fd, "Finalizing Margo ...\n");
    margo_finalize(margo_mid);

    DAEMON_DEBUG0(debug_fd, "Finalizing Argobots ...\n");
    ABT_finalize();

    DAEMON_DEBUG0(debug_fd, "Destroying Mercury context ...\n");
    HG_Context_destroy(mercury_hg_context);
    DAEMON_DEBUG0(debug_fd, "Finalizing Mercury class ...\n");
    HG_Finalize(mercury_hg_class);
    DAEMON_DEBUG0(debug_fd, "Preload library shut down.\n");
#endif

    fclose(debug_fd);
}