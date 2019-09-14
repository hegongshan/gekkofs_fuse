/*
  Copyright 2018-2019, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2019, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#include <global/log_util.hpp>
#include <global/env_util.hpp>
#include <global/path_util.hpp>
#include <global/global_defs.hpp>
#include <global/configure.hpp>
#include <client/preload.hpp>
#include <client/resolve.hpp>
#include <global/rpc/distributor.hpp>
#include "global/rpc/rpc_types.hpp"
#include <client/rpc/ld_rpc_management.hpp>
#include <client/preload_util.hpp>
#include <client/intercept.hpp>
#include <client/rpc/hg_rpcs.hpp>
#include <hermes.hpp>

#include <sys/types.h>
#include <dirent.h>

#include <fstream>


using namespace std;

// make sure that things are only initialized once
static pthread_once_t init_env_thread = PTHREAD_ONCE_INIT;

// RPC IDs
hg_id_t rpc_config_id;
hg_id_t rpc_mk_node_id;
hg_id_t rpc_stat_id;
hg_id_t rpc_rm_node_id;
hg_id_t rpc_decr_size_id;
hg_id_t rpc_update_metadentry_id;
hg_id_t rpc_get_metadentry_size_id;
hg_id_t rpc_update_metadentry_size_id;
hg_id_t rpc_mk_symlink_id;
hg_id_t rpc_write_data_id;
hg_id_t rpc_read_data_id;
hg_id_t rpc_trunc_data_id;
hg_id_t rpc_get_dirents_id;
hg_id_t rpc_chunk_stat_id;

std::unique_ptr<hermes::async_engine> ld_network_service;

static inline void exit_error_msg(int errcode, const string& msg) {
    CTX->log()->error(msg);
    cerr << "GekkoFS error: " << msg << endl;
    exit(errcode);
}

/**
 * Initializes the Hermes client for a given transport prefix
 * @param transport_prefix
 * @return true if succesfully initialized; false otherwise
 */
bool init_hermes_client(const std::string& transport_prefix) {

    try {

        hermes::engine_options opts;
#if USE_SHM
        opts |= hermes::use_auto_sm;
#endif

        ld_network_service = 
            std::make_unique<hermes::async_engine>(
                    hermes::get_transport_type(transport_prefix), opts);
        ld_network_service->run();
    } catch (const std::exception& ex) {
        fmt::print(stderr, "Failed to initialize Hermes RPC client {}\n", 
                   ex.what());
        return false;
    }

    rpc_config_id = gkfs::rpc::fs_config::public_id;
    rpc_mk_node_id = gkfs::rpc::create::public_id;
    rpc_stat_id = gkfs::rpc::stat::public_id;
    rpc_rm_node_id = gkfs::rpc::remove::public_id;
    rpc_decr_size_id = gkfs::rpc::decr_size::public_id;
    rpc_update_metadentry_id = gkfs::rpc::update_metadentry::public_id;
    rpc_get_metadentry_size_id = gkfs::rpc::get_metadentry_size::public_id;
    rpc_update_metadentry_size_id = gkfs::rpc::update_metadentry::public_id;

#ifdef HAS_SYMLINKS
    rpc_mk_symlink_id = gkfs::rpc::mk_symlink::public_id;
#endif // HAS_SYMLINKS

    rpc_write_data_id = gkfs::rpc::write_data::public_id;
    rpc_read_data_id = gkfs::rpc::read_data::public_id;
    rpc_trunc_data_id = gkfs::rpc::trunc_data::public_id;
    rpc_get_dirents_id = gkfs::rpc::get_dirents::public_id;
    rpc_chunk_stat_id = gkfs::rpc::chunk_stat::public_id;

    return true;
}

static inline std::set<int>
query_open_fds() {

    std::set<int> fds;
    const std::string path{"/proc/self/fd"};

    std::unique_ptr<DIR, decltype(&::closedir)> dirp(
            ::opendir(path.c_str()), 
            closedir);

    struct dirent entry;
    struct dirent *result;

    while (::readdir_r(dirp.get(), &entry, &result) == 0 && result != NULL) {
        const std::string name{entry.d_name};

        if(name == "." || name == ".." || 
           std::stoi(name) == dirfd(dirp.get())) {
            continue;
        }

        fds.insert(std::stoi(name));
    }

    return fds;
}


/**
 * This function is only called in the preload constructor and initializes 
 * the file system client
 */
void init_ld_environment_() {

    // Client applications such as ssh attempt to close all open file 
    // descriptors, which causes havoc with the interception library's internal 
    // state. To account for this, in the interception code we keep track of
    // internal fds by distinguishing between internal syscalls (i.e. those
    // coming from internal code) application syscalls. The problem is that
    // at this point in initialization we have not enabled interception yet,
    // but the initialization process itself needs to create file descriptors.
    // To solve this problem, we find out which fds are created by the 
    // initialization process and manually protect them at this point
    auto pre_init_fds = query_open_fds();

    // initialize Hermes interface to Mercury
    if (!init_hermes_client(RPC_PROTOCOL)) {
        exit_error_msg(EXIT_FAILURE, "Unable to initialize Hermes RPC client");
    }

    try {
        load_hosts();
    } catch (const std::exception& e) {
        exit_error_msg(EXIT_FAILURE, "Failed to load hosts addresses: "s + e.what());
    }

    /* Setup distributor */
    auto simple_hash_dist = std::make_shared<SimpleHashDistributor>(CTX->local_host_id(), CTX->hosts().size());
    CTX->distributor(simple_hash_dist);

    if (!rpc_send::get_fs_config()) {
        exit_error_msg(EXIT_FAILURE, "Unable to fetch file system configurations from daemon process through RPC.");
    }

    auto post_init_fds = query_open_fds();
    std::set<int> internal_fds{3}; // fd 3 is created by the logging system

    std::set_difference(post_init_fds.begin(), post_init_fds.end(),
                        pre_init_fds.begin(), pre_init_fds.end(),
                        std::inserter(internal_fds, internal_fds.end()));

    for(const auto& fd : internal_fds) {
        CTX->register_internal_fd(fd);
    }

    CTX->log()->info("{}() Environment initialization successful.", __func__);
}

void init_ld_env_if_needed() {
    pthread_once(&init_env_thread, init_ld_environment_);
}

void init_logging() {
    std::string path;
    try {
        path = gkfs::get_env_own("PRELOAD_LOG_PATH");
    } catch (const std::exception& e) {
        path = DEFAULT_PRELOAD_LOG_PATH;
    }

    spdlog::level::level_enum level;
    try {
        level = get_spdlog_level(gkfs::get_env_own("LOG_LEVEL"));
    } catch (const std::exception& e) {
        level = get_spdlog_level(DEFAULT_DAEMON_LOG_LEVEL);
    }

    auto logger_names = std::vector<std::string> {"main"};

    setup_loggers(logger_names, level, path);

    CTX->log(spdlog::get(logger_names.at(0)));
}

void log_prog_name() {
    std::string line;
    std::ifstream cmdline("/proc/self/cmdline");
    if (!cmdline.is_open()) {
        CTX->log()->error("Unable to open cmdline file");
        throw std::runtime_error("Unable to open cmdline file");
    }
    if(!getline(cmdline, line)) {
        throw std::runtime_error("Unable to read cmdline file");
    }
    std::replace(line.begin(), line.end(), '\0', ' ');
    CTX->log()->info("Command to itercept: '{}'", line);
    cmdline.close();
}

/**
 * Called initially ONCE when preload library is used with the LD_PRELOAD environment variable
 */
void init_preload() {

    init_logging();
    CTX->log()->debug("Initialized logging subsystem");
    log_prog_name();
    init_cwd();
    CTX->log()->debug("Current working directory: '{}'", CTX->cwd());
    init_ld_env_if_needed();
    CTX->enable_interception();
    CTX->log()->debug("{}() exit", __func__);
    start_interception();
}

/**
 * Called last when preload library is used with the LD_PRELOAD environment variable
 */
void destroy_preload() {

    stop_interception();
    CTX->disable_interception();

    CTX->clear_hosts();
    CTX->log()->debug("{}() About to finalize the Hermes RPC client", __func__);

    ld_network_service.reset();

    CTX->log()->debug("{}() Shut down Hermes RPC client successful", __func__);
    CTX->log()->info("All services shut down. Client shutdown complete.");
}
