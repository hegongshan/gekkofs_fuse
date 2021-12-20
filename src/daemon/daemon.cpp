/*
  Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  This file is part of GekkoFS.

  GekkoFS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  GekkoFS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with GekkoFS.  If not, see <https://www.gnu.org/licenses/>.

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <daemon/daemon.hpp>
#include <version.hpp>
#include <common/log_util.hpp>
#include <common/env_util.hpp>
#include <common/rpc/rpc_types.hpp>
#include <common/rpc/rpc_util.hpp>

#include <daemon/env.hpp>
#include <daemon/handler/rpc_defs.hpp>
#include <daemon/ops/metadentry.hpp>
#include <daemon/backend/metadata/db.hpp>
#include <daemon/backend/data/chunk_storage.hpp>
#include <daemon/util.hpp>
#include <CLI11/CLI11.hpp>

#ifdef GKFS_ENABLE_AGIOS
#include <daemon/scheduler/agios.hpp>
#endif


#include <filesystem>
#include <iostream>
#include <fstream>
#include <csignal>
#include <condition_variable>

extern "C" {
#include <unistd.h>
#include <cstdlib>
}

using namespace std;
namespace fs = std::filesystem;

static condition_variable shutdown_please;
static mutex mtx;

struct cli_options {
    string mountdir;
    string rootdir;
    string metadir;
    string listen;
    string hosts_file;
    string rpc_protocol;
};

void
init_io_tasklet_pool() {
    static_assert(gkfs::config::rpc::daemon_io_xstreams >= 0,
                  "Daemon IO Execution streams must be higher than 0!");
    unsigned int xstreams_num = gkfs::config::rpc::daemon_io_xstreams;

    // retrieve the pool of the just created scheduler
    ABT_pool pool;
    auto ret = ABT_pool_create_basic(ABT_POOL_FIFO_WAIT, ABT_POOL_ACCESS_MPMC,
                                     ABT_TRUE, &pool);
    if(ret != ABT_SUCCESS) {
        throw runtime_error("Failed to create I/O tasks pool");
    }

    // create all subsequent xstream and the associated scheduler, all tapping
    // into the same pool
    vector<ABT_xstream> xstreams(xstreams_num);
    for(unsigned int i = 0; i < xstreams_num; ++i) {
        ret = ABT_xstream_create_basic(ABT_SCHED_BASIC_WAIT, 1, &pool,
                                       ABT_SCHED_CONFIG_NULL, &xstreams[i]);
        if(ret != ABT_SUCCESS) {
            throw runtime_error(
                    "Failed to create task execution streams for I/O operations");
        }
    }

    RPC_DATA->io_streams(xstreams);
    RPC_DATA->io_pool(pool);
}

/**
 * Registers RPC handlers to Margo instance
 * @param hg_class
 */
void
register_server_rpcs(margo_instance_id mid) {
    MARGO_REGISTER(mid, gkfs::rpc::tag::fs_config, void, rpc_config_out_t,
                   rpc_srv_get_fs_config);
    MARGO_REGISTER(mid, gkfs::rpc::tag::create, rpc_mk_node_in_t, rpc_err_out_t,
                   rpc_srv_create);
    MARGO_REGISTER(mid, gkfs::rpc::tag::stat, rpc_path_only_in_t,
                   rpc_stat_out_t, rpc_srv_stat);
    MARGO_REGISTER(mid, gkfs::rpc::tag::decr_size, rpc_trunc_in_t,
                   rpc_err_out_t, rpc_srv_decr_size);
    MARGO_REGISTER(mid, gkfs::rpc::tag::remove_metadata, rpc_rm_node_in_t,
                   rpc_rm_metadata_out_t, rpc_srv_remove_metadata);
    MARGO_REGISTER(mid, gkfs::rpc::tag::remove_data, rpc_rm_node_in_t,
                   rpc_err_out_t, rpc_srv_remove_data);
    MARGO_REGISTER(mid, gkfs::rpc::tag::update_metadentry,
                   rpc_update_metadentry_in_t, rpc_err_out_t,
                   rpc_srv_update_metadentry);
    MARGO_REGISTER(mid, gkfs::rpc::tag::get_metadentry_size, rpc_path_only_in_t,
                   rpc_get_metadentry_size_out_t, rpc_srv_get_metadentry_size);
    MARGO_REGISTER(mid, gkfs::rpc::tag::update_metadentry_size,
                   rpc_update_metadentry_size_in_t,
                   rpc_update_metadentry_size_out_t,
                   rpc_srv_update_metadentry_size);
    MARGO_REGISTER(mid, gkfs::rpc::tag::get_dirents, rpc_get_dirents_in_t,
                   rpc_get_dirents_out_t, rpc_srv_get_dirents);
    MARGO_REGISTER(mid, gkfs::rpc::tag::get_dirents_extended,
                   rpc_get_dirents_in_t, rpc_get_dirents_out_t,
                   rpc_srv_get_dirents_extended);
#ifdef HAS_SYMLINKS
    MARGO_REGISTER(mid, gkfs::rpc::tag::mk_symlink, rpc_mk_symlink_in_t,
                   rpc_err_out_t, rpc_srv_mk_symlink);
#endif
    MARGO_REGISTER(mid, gkfs::rpc::tag::write, rpc_write_data_in_t,
                   rpc_data_out_t, rpc_srv_write);
    MARGO_REGISTER(mid, gkfs::rpc::tag::read, rpc_read_data_in_t,
                   rpc_data_out_t, rpc_srv_read);
    MARGO_REGISTER(mid, gkfs::rpc::tag::truncate, rpc_trunc_in_t, rpc_err_out_t,
                   rpc_srv_truncate);
    MARGO_REGISTER(mid, gkfs::rpc::tag::get_chunk_stat, rpc_chunk_stat_in_t,
                   rpc_chunk_stat_out_t, rpc_srv_get_chunk_stat);
}

void
init_rpc_server() {
    hg_addr_t addr_self = nullptr;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];
    struct hg_init_info hg_options = HG_INIT_INFO_INITIALIZER;
    hg_options.auto_sm = GKFS_DATA->use_auto_sm() ? HG_TRUE : HG_FALSE;
    hg_options.stats = HG_FALSE;
    if(gkfs::rpc::protocol::ofi_psm2 == GKFS_DATA->rpc_protocol())
        hg_options.na_init_info.progress_mode = NA_NO_BLOCK;
    // Start Margo (this will also initialize Argobots and Mercury internally)
    auto margo_config = fmt::format(
            R"({{ "use_progress_thread" : true, "rpc_thread_count" : {} }})",
            gkfs::config::rpc::daemon_handler_xstreams);
    struct margo_init_info args = {nullptr};
    args.json_config = margo_config.c_str();
    args.hg_init_info = &hg_options;
    auto* mid = margo_init_ext(GKFS_DATA->bind_addr().c_str(),
                               MARGO_SERVER_MODE, &args);

    if(mid == MARGO_INSTANCE_NULL) {
        throw runtime_error("Failed to initialize the Margo RPC server");
    }
    // Figure out what address this server is listening on (must be freed when
    // finished)
    auto hret = margo_addr_self(mid, &addr_self);
    if(hret != HG_SUCCESS) {
        margo_finalize(mid);
        throw runtime_error("Failed to retrieve server RPC address");
    }
    // Convert the address to a cstring (with \0 terminator).
    hret = margo_addr_to_string(mid, addr_self_cstring, &addr_self_cstring_sz,
                                addr_self);
    if(hret != HG_SUCCESS) {
        margo_addr_free(mid, addr_self);
        margo_finalize(mid);
        throw runtime_error("Failed to convert server RPC address to string");
    }
    margo_addr_free(mid, addr_self);

    std::string addr_self_str(addr_self_cstring);
    RPC_DATA->self_addr_str(addr_self_str);

    GKFS_DATA->spdlogger()->info("{}() Accepting RPCs on address {}", __func__,
                                 addr_self_cstring);

    // Put context and class into RPC_data object
    RPC_DATA->server_rpc_mid(mid);

    // register RPCs
    register_server_rpcs(mid);
}

void
init_environment() {
    // Initialize metadata db
    std::string metadata_path = GKFS_DATA->metadir() + "/rocksdb"s;
    GKFS_DATA->spdlogger()->debug("{}() Initializing metadata DB: '{}'",
                                  __func__, metadata_path);
    try {
        GKFS_DATA->mdb(
                std::make_shared<gkfs::metadata::MetadataDB>(metadata_path));
    } catch(const std::exception& e) {
        GKFS_DATA->spdlogger()->error(
                "{}() Failed to initialize metadata DB: {}", __func__,
                e.what());
        throw;
    }

    GKFS_DATA->spdlogger()->debug("{}() Initializing Distributor ", __func__);
    try {
#ifdef GKFS_USE_GUIDED_DISTRIBUTION
        auto distributor = std::make_shared<gkfs::rpc::GuidedDistributor>();
#else
        auto distributor = std::make_shared<gkfs::rpc::SimpleHashDistributor>();
#endif
        RPC_DATA->distributor(distributor);
    } catch(const std::exception& e) {
        GKFS_DATA->spdlogger()->error(
                "{}() Failed to initialize Distributor: {}", __func__,
                e.what());
        throw;
    }

#ifdef GKFS_ENABLE_FORWARDING
    GKFS_DATA->spdlogger()->debug("{}() Enable I/O forwarding mode", __func__);
#endif

#ifdef GKFS_ENABLE_AGIOS
    // Initialize AGIOS scheduler
    GKFS_DATA->spdlogger()->debug("{}() Initializing AGIOS scheduler: '{}'",
                                  __func__, "/tmp/agios.conf");
    try {
        agios_initialize();
    } catch(const std::exception& e) {
        GKFS_DATA->spdlogger()->error(
                "{}() Failed to initialize AGIOS scheduler: {}", __func__,
                e.what());
        throw;
    }
#endif

    // Initialize data backend
    std::string chunk_storage_path = GKFS_DATA->rootdir() + "/data/chunks"s;
    GKFS_DATA->spdlogger()->debug("{}() Initializing storage backend: '{}'",
                                  __func__, chunk_storage_path);
    fs::create_directories(chunk_storage_path);
    try {
        GKFS_DATA->storage(std::make_shared<gkfs::data::ChunkStorage>(
                chunk_storage_path, gkfs::config::rpc::chunksize));
    } catch(const std::exception& e) {
        GKFS_DATA->spdlogger()->error(
                "{}() Failed to initialize storage backend: {}", __func__,
                e.what());
        throw;
    }

    // Init margo for RPC
    GKFS_DATA->spdlogger()->debug("{}() Initializing RPC server: '{}'",
                                  __func__, GKFS_DATA->bind_addr());
    try {
        init_rpc_server();
    } catch(const std::exception& e) {
        GKFS_DATA->spdlogger()->error(
                "{}() Failed to initialize RPC server: {}", __func__, e.what());
        throw;
    }

    // Init Argobots ESs to drive IO
    try {
        GKFS_DATA->spdlogger()->debug("{}() Initializing I/O pool", __func__);
        init_io_tasklet_pool();
    } catch(const std::exception& e) {
        GKFS_DATA->spdlogger()->error(
                "{}() Failed to initialize Argobots pool for I/O: {}", __func__,
                e.what());
        throw;
    }

    // TODO set metadata configurations. these have to go into a user
    // configurable file that is parsed here
    GKFS_DATA->atime_state(gkfs::config::metadata::use_atime);
    GKFS_DATA->mtime_state(gkfs::config::metadata::use_mtime);
    GKFS_DATA->ctime_state(gkfs::config::metadata::use_ctime);
    GKFS_DATA->link_cnt_state(gkfs::config::metadata::use_link_cnt);
    GKFS_DATA->blocks_state(gkfs::config::metadata::use_blocks);
    // Create metadentry for root directory
    gkfs::metadata::Metadata root_md{S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO};
    try {
        gkfs::metadata::create("/", root_md);
    } catch(const std::exception& e) {
        throw runtime_error("Failed to write root metadentry to KV store: "s +
                            e.what());
    }
    // setup hostfile to let clients know that a daemon is running on this host
    if(!GKFS_DATA->hosts_file().empty()) {
        gkfs::utils::populate_hosts_file();
    }
    GKFS_DATA->spdlogger()->info("Startup successful. Daemon is ready.");
}

#ifdef GKFS_ENABLE_AGIOS
/**
 * Initialize the AGIOS scheduling library
 */
void
agios_initialize() {
    char configuration[] = "/tmp/agios.conf";

    if(!agios_init(NULL, NULL, configuration, 0)) {
        GKFS_DATA->spdlogger()->error(
                "{}() Failed to initialize AGIOS scheduler: '{}'", __func__,
                configuration);

        agios_exit();

        throw;
    }
}
#endif

/**
 * Destroys the margo, argobots, and mercury environments
 */
void
destroy_enviroment() {
    GKFS_DATA->spdlogger()->debug("{}() Removing mount directory", __func__);
    std::error_code ecode;
    fs::remove_all(GKFS_DATA->mountdir(), ecode);
    GKFS_DATA->spdlogger()->debug("{}() Freeing I/O executions streams",
                                  __func__);
    for(unsigned int i = 0; i < RPC_DATA->io_streams().size(); i++) {
        ABT_xstream_join(RPC_DATA->io_streams().at(i));
        ABT_xstream_free(&RPC_DATA->io_streams().at(i));
    }

    if(!GKFS_DATA->hosts_file().empty()) {
        GKFS_DATA->spdlogger()->debug("{}() Removing hosts file", __func__);
        try {
            gkfs::utils::destroy_hosts_file();
        } catch(const fs::filesystem_error& e) {
            GKFS_DATA->spdlogger()->debug("{}() hosts file not found",
                                          __func__);
        }
    }

    if(RPC_DATA->server_rpc_mid() != nullptr) {
        GKFS_DATA->spdlogger()->debug("{}() Finalizing margo RPC server",
                                      __func__);
        margo_finalize(RPC_DATA->server_rpc_mid());
    }

    GKFS_DATA->spdlogger()->info("{}() Closing metadata DB", __func__);
    GKFS_DATA->close_mdb();
}

void
shutdown_handler(int dummy) {
    GKFS_DATA->spdlogger()->info("{}() Received signal: '{}'", __func__,
                                 strsignal(dummy));
    shutdown_please.notify_all();
}

void
initialize_loggers() {
    std::string path = gkfs::config::log::daemon_log_path;
    // Try to get log path from env variable
    std::string env_path_key = DAEMON_ENV_PREFIX;
    env_path_key += "DAEMON_LOG_PATH";
    char* env_path = getenv(env_path_key.c_str());
    if(env_path != nullptr) {
        path = env_path;
    }

    spdlog::level::level_enum level =
            gkfs::log::get_level(gkfs::config::log::daemon_log_level);
    // Try to get log path from env variable
    std::string env_level_key = DAEMON_ENV_PREFIX;
    env_level_key += "LOG_LEVEL";
    char* env_level = getenv(env_level_key.c_str());
    if(env_level != nullptr) {
        level = gkfs::log::get_level(env_level);
    }

    auto logger_names = std::vector<std::string>{
            "main",
            "MetadataModule",
            "DataModule",
    };

    gkfs::log::setup(logger_names, level, path);
}

/**
 * Parses input of user
 * @param vm
 * @throws runtime_error
 */
void
parse_input(const cli_options& opts, const CLI::App& desc) {
    auto rpc_protocol = string(gkfs::rpc::protocol::ofi_sockets);
    if(desc.count("--rpc-protocol")) {
        rpc_protocol = opts.rpc_protocol;
        if(rpc_protocol != gkfs::rpc::protocol::ofi_verbs &&
           rpc_protocol != gkfs::rpc::protocol::ofi_sockets &&
           rpc_protocol != gkfs::rpc::protocol::ofi_psm2) {
            throw runtime_error(fmt::format(
                    "Given RPC protocol '{}' not supported. Check --help for supported protocols.",
                    rpc_protocol));
        }
    }

    auto use_auto_sm = desc.count("--auto-sm") != 0;
    GKFS_DATA->use_auto_sm(use_auto_sm);
    GKFS_DATA->spdlogger()->debug(
            "{}() Shared memory (auto_sm) for intra-node communication (IPCs) set to '{}'.",
            __func__, use_auto_sm);

    string addr{};
    if(desc.count("--listen")) {
        addr = opts.listen;
        // ofi+verbs requires an empty addr to bind to the ib interface
        if(rpc_protocol == gkfs::rpc::protocol::ofi_verbs) {
            /*
             * FI_VERBS_IFACE : The prefix or the full name of the network
             * interface associated with the verbs device (default: ib) Mercury
             * does not allow to bind to an address when ofi+verbs is used
             */
            if(!secure_getenv("FI_VERBS_IFACE"))
                setenv("FI_VERBS_IFACE", addr.c_str(), 1);
            addr = ""s;
        }
    } else {
        if(rpc_protocol != gkfs::rpc::protocol::ofi_verbs)
            addr = gkfs::rpc::get_my_hostname(true);
    }

    GKFS_DATA->rpc_protocol(rpc_protocol);
    GKFS_DATA->bind_addr(fmt::format("{}://{}", rpc_protocol, addr));

    string hosts_file;
    if(desc.count("--hosts-file")) {
        hosts_file = opts.hosts_file;
    } else {
        hosts_file = gkfs::env::get_var(gkfs::env::HOSTS_FILE,
                                        gkfs::config::hostfile_path);
    }
    GKFS_DATA->hosts_file(hosts_file);

    assert(desc.count("--mountdir"));
    auto mountdir = opts.mountdir;
    // Create mountdir. We use this dir to get some information on the
    // underlying fs with statfs in gkfs_statfs
    fs::create_directories(mountdir);
    GKFS_DATA->mountdir(fs::canonical(mountdir).native());

    assert(desc.count("--rootdir"));
    auto rootdir = opts.rootdir;

#ifdef GKFS_ENABLE_FORWARDING
    // In forwarding mode, the backend is shared
    auto rootdir_path = fs::path(rootdir);
#else
    auto rootdir_path = fs::path(rootdir) / fmt::format_int(getpid()).str();
#endif

    if(desc.count("--clean-rootdir")) {
        // may throw exception (caught in main)
        GKFS_DATA->spdlogger()->debug("{}() Cleaning rootdir '{}' ...",
                                      __func__, rootdir_path.native());
        fs::remove_all(rootdir_path.native());
        GKFS_DATA->spdlogger()->info("{}() Rootdir cleaned.", __func__);
    }
    GKFS_DATA->spdlogger()->debug("{}() Root directory: '{}'", __func__,
                                  rootdir_path.native());
    fs::create_directories(rootdir_path);
    GKFS_DATA->rootdir(rootdir_path.native());

    if(desc.count("--metadir")) {
        auto metadir = opts.metadir;

#ifdef GKFS_ENABLE_FORWARDING
        auto metadir_path = fs::path(metadir) / fmt::format_int(getpid()).str();
#else
        auto metadir_path = fs::path(metadir);
#endif

        fs::create_directories(metadir_path);
        GKFS_DATA->metadir(fs::canonical(metadir_path).native());

        GKFS_DATA->spdlogger()->debug("{}() Meta directory: '{}'", __func__,
                                      metadir_path.native());
    } else {
        // use rootdir as metadata dir
        auto metadir = opts.rootdir;

#ifdef GKFS_ENABLE_FORWARDING
        auto metadir_path = fs::path(metadir) / fmt::format_int(getpid()).str();
        fs::create_directories(metadir_path);
        GKFS_DATA->metadir(fs::canonical(metadir_path).native());
#else
        GKFS_DATA->metadir(GKFS_DATA->rootdir());
#endif
    }
}

int
main(int argc, const char* argv[]) {
    // Define arg parsing
    CLI::App desc{"Allowed options"};
    cli_options opts{};
    // clang-format off
    desc.add_option("--mountdir,-m", opts.mountdir,
                    "Virtual mounting directory where GekkoFS is available.")
                    ->required()
                    ->expected(1);
    desc.add_option(
                    "--rootdir,-r", opts.rootdir,
                    "Local data directory where GekkoFS data for this daemon is stored.")
                    ->required()
                    ->expected(1);
    desc.add_option(
                    "--metadir,-i", opts.metadir,
                    "Metadata directory where GekkoFS' RocksDB data directory is located. If not set, rootdir is used.");
    desc.add_option(
                    "--listen,-l", opts.listen,
                    "Address or interface to bind the daemon to. Default: local hostname.\n"
                    "When used with ofi+verbs the FI_VERBS_IFACE environment variable is set accordingly "
                    "which associates the verbs device with the network interface. In case FI_VERBS_IFACE "
                    "is already defined, the argument is ignored. Default 'ib'.");
    desc.add_option("--hosts-file,-H", opts.hosts_file,
                    "Shared file used by deamons to register their "
                    "endpoints. (default './gkfs_hosts.txt')");
    desc.add_option(
                    "--rpc-protocol,-P", opts.rpc_protocol,
                    "Used RPC protocol for inter-node communication.\n"
                    "Available: {ofi+sockets, ofi+verbs, ofi+psm2} for TCP, Infiniband, "
                    "and Omni-Path, respectively. (Default ofi+sockets)\n"
                    "Libfabric must have enabled support verbs or psm2.");
    desc.add_flag(
                "--auto-sm",
                "Enables intra-node communication (IPCs) via the `na+sm` (shared memory) protocol, "
                "instead of using the RPC protocol. (Default off)");
    desc.add_flag(
                "--clean-rootdir",
                "Cleans Rootdir >before< launching the deamon");
    desc.add_flag("--version", "Print version and exit.");
    // clang-format on
    try {
        desc.parse(argc, argv);
    } catch(const CLI::ParseError& e) {
        return desc.exit(e);
    }
    if(desc.count("--version")) {
        cout << GKFS_VERSION_STRING << endl;
#ifndef NDEBUG
        cout << "Debug: ON" << endl;
#else
        cout << "Debug: OFF" << endl;
#endif
#if CREATE_CHECK_PARENTS
        cout << "Create check parents: ON" << endl;
#else
        cout << "Create check parents: OFF" << endl;
#endif
        cout << "Chunk size: " << gkfs::config::rpc::chunksize << " bytes"
             << endl;
        return 0;
    }
    // intitialize logging framework
    initialize_loggers();
    GKFS_DATA->spdlogger(spdlog::get("main"));

    // parse all input parameters and populate singleton structures
    try {
        parse_input(opts, desc);
    } catch(const std::exception& e) {
        cerr << fmt::format("Parsing arguments failed: '{}'. Exiting.",
                            e.what());
        exit(EXIT_FAILURE);
    }

    /*
     * Initialize environment and start daemon. Wait until signaled to cancel
     * before shutting down
     */
    try {
        GKFS_DATA->spdlogger()->info("{}() Initializing environment", __func__);
        init_environment();
    } catch(const std::exception& e) {
        auto emsg =
                fmt::format("Failed to initialize environment: {}", e.what());
        GKFS_DATA->spdlogger()->error(emsg);
        cerr << emsg << endl;
        destroy_enviroment();
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, shutdown_handler);
    signal(SIGTERM, shutdown_handler);
    signal(SIGKILL, shutdown_handler);

    unique_lock<mutex> lk(mtx);
    // Wait for shutdown signal to initiate shutdown protocols
    shutdown_please.wait(lk);
    GKFS_DATA->spdlogger()->info("{}() Shutting down...", __func__);
    destroy_enviroment();
    GKFS_DATA->spdlogger()->info("{}() Complete. Exiting...", __func__);
    return 0;
}
