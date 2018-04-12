/**
 * All intercepted functions are mapped to a different function pointer prefixing <libc_>
 */
#include <preload/passthrough.hpp>

#include <dlfcn.h>

static pthread_once_t init_lib_thread = PTHREAD_ONCE_INIT;

// external variables that are initialized here
std::shared_ptr<spdlog::logger> ld_logger;
std::shared_ptr<FsConfig> fs_config;

// function pointer for preloading
void* libc;

void* libc_open;
void* libc_fopen; // XXX Does not work with streaming pointers. If used will block forever
void* libc_fopen64; // XXX Does not work with streaming pointers. If used will block forever

void* libc_mkdir;
void* libc_mkdirat;
void* libc_unlink;
void* libc_rmdir;

void* libc_close;

void* libc_access;
void* libc_faccessat;

void* libc_stat;
void* libc_fstat;
void* libc_lstat;
void* libc___xstat;
void* libc___xstat64;
void* libc___fxstat;
void* libc___fxstat64;
void* libc___lxstat;
void* libc___lxstat64;

void* libc_statfs;
void* libc_fstatfs;

void* libc_puts;

void* libc_write;
void* libc_pwrite;
void* libc_pwrite64;
void* libc_read;
void* libc_pread;
void* libc_pread64;

void* libc_lseek;
void* libc_lseek64;

void* libc_fsync;
void* libc_fdatasync;

void* libc_truncate;
void* libc_ftruncate;

void* libc_dup;
void* libc_dup2;
void* libc_dup3;


void init_passthrough_() {
    libc = dlopen("libc.so.6", RTLD_LAZY);
    if(libc == nullptr){
        std::cerr << "ERROR: failed opening glibc shared object: " << dlerror() << std::endl;
        exit(EXIT_FAILURE);
    }

    libc_open = dlsym(libc, "open");
//    libc_fopen = dlsym(libc, "fopen");
//    libc_fopen64 = dlsym(libc, "fopen64");
    libc_mkdir = dlsym(libc, "mkdir");
    libc_mkdirat = dlsym(libc, "mkdirat");

    libc_unlink = dlsym(libc, "unlink");
    libc_rmdir = dlsym(libc, "rmdir");

    libc_close = dlsym(libc, "close");

    libc_access = dlsym(libc, "access");
    libc_faccessat = dlsym(libc, "faccessat");

    libc_stat = dlsym(libc, "stat");
    libc_fstat = dlsym(libc, "fstat");
    libc_lstat = dlsym(libc, "lstat");
    libc___xstat = dlsym(libc, "__xstat");
    libc___xstat64 = dlsym(libc, "__xstat64");
    libc___fxstat = dlsym(libc, "__fxstat");
    libc___fxstat64 = dlsym(libc, "__fxstat64");
    libc___lxstat = dlsym(libc, "__lxstat");
    libc___lxstat64 = dlsym(libc, "__lxstat64");

    libc_statfs = dlsym(libc, "statfs");
    libc_fstatfs = dlsym(libc, "fstatfs");

    libc_puts = dlsym(libc, "puts");

    libc_write = dlsym(libc, "write");
    libc_pwrite = dlsym(libc, "pwrite");
    libc_pwrite64 = dlsym(libc, "pwrite64");
    libc_read = dlsym(libc, "read");
    libc_pread = dlsym(libc, "pread");
    libc_pread64 = dlsym(libc, "pread64");

    libc_lseek = dlsym(libc, "lseek");
    libc_lseek64 = dlsym(libc, "lseek64");
    libc_fsync = dlsym(libc, "fsync");
    libc_fdatasync = dlsym(libc, "fdatasync");

    libc_truncate = dlsym(libc, "truncate");
    libc_ftruncate = dlsym(libc, "ftruncate");

    libc_dup = dlsym(libc, "dup");
    libc_dup2 = dlsym(libc, "dup2");
    libc_dup3 = dlsym(libc, "dup3");

    fs_config = std::make_shared<struct FsConfig>();
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

    ld_logger->debug("{}() Passthrough initialized.", __func__);
}

void init_passthrough_if_needed() {
    pthread_once(&init_lib_thread, init_passthrough_);
}