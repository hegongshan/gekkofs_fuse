
#include <daemon/adafs_daemon.hpp>
#include <global/rpc/ipc_types.hpp>
#include <global/rpc/rpc_types.hpp>
#include <daemon/handler/rpc_defs.hpp>
#include <daemon/db/db_util.hpp>
#include <daemon/adafs_ops/metadentry.hpp>

#include <csignal>
#include <condition_variable>

using namespace std;
namespace po = boost::program_options;

static condition_variable shutdown_please;
static mutex mtx;

bool init_environment() {
    // Initialize rocksdb
    if (!init_rocksdb()) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to initialize RocksDB.", __func__);
        return false;
    }
    // init margo
    if (!init_rpc_server()) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to initialize Margo RPC server.", __func__);
        return false;
    }
    if (!init_ipc_server()) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to initialize Margo IPC server.", __func__);
        return false;
    }
    if (!init_io_tasklet_pool()) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to initialize Argobots pool for I/O.", __func__);
        return false;
    }
    // Register daemon to system
    if (!register_daemon_proc()) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to register the daemon process to the system.", __func__);
        return false;
    }
    // TODO set metadata configurations. these have to go into a user configurable file that is parsed here
    ADAFS_DATA->atime_state(MDATA_USE_ATIME);
    ADAFS_DATA->mtime_state(MDATA_USE_MTIME);
    ADAFS_DATA->ctime_state(MDATA_USE_CTIME);
    ADAFS_DATA->uid_state(MDATA_USE_UID);
    ADAFS_DATA->gid_state(MDATA_USE_GID);
    ADAFS_DATA->inode_no_state(MDATA_USE_INODE_NO);
    ADAFS_DATA->link_cnt_state(MDATA_USE_LINK_CNT);
    ADAFS_DATA->blocks_state(MDATA_USE_BLOCKS);
    // Create metadentry for root directory
    if (create_metadentry(ADAFS_DATA->mountdir(), S_IFDIR | 777) != 0) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to write root metadentry to KV store.", __func__);
        return false;
    }
    ADAFS_DATA->spdlogger()->info("Startup successful. Daemon is ready.");
    return true;
}

/**
 * Destroys the margo, argobots, and mercury environments
 */
void destroy_enviroment() {
#ifdef MARGODIAG
    cout << "\n####################\n\nMargo IPC server stats: " << endl;
    margo_diag_dump(RPC_DATA->server_ipc_mid(), "-", 0);
    cout << "\n####################\n\nMargo RPC server stats: " << endl;
    margo_diag_dump(RPC_DATA->server_rpc_mid(), "-", 0);
#endif
    bfs::remove_all(ADAFS_DATA->mountdir());
    for (unsigned int i = 0; i < RPC_DATA->io_streams().size(); i++) {
        ABT_xstream_join(RPC_DATA->io_streams().at(i));
        ABT_xstream_free(&RPC_DATA->io_streams().at(i));
    }
    ADAFS_DATA->spdlogger()->info("{}() Freeing I/O executions streams successful", __func__);
    if (!deregister_daemon_proc())
        ADAFS_DATA->spdlogger()->warn("{}() Unable to clean up auxiliary files", __func__);
    else
        ADAFS_DATA->spdlogger()->debug("{}() Cleaning auxiliary files successful", __func__);
    // The shutdown order is important because the RPC server is started first, it has to be stopped last due to Argobots
    if (RPC_DATA->server_ipc_mid() != nullptr) {
        margo_finalize(RPC_DATA->server_ipc_mid());
        ADAFS_DATA->spdlogger()->info("{}() Margo IPC server shut down successful", __func__);
    }
    if (RPC_DATA->server_rpc_mid() != nullptr) {
        margo_finalize(RPC_DATA->server_rpc_mid());
        ADAFS_DATA->spdlogger()->info("{}() Margo RPC server shut down successful", __func__);
    }
    ADAFS_DATA->spdlogger()->info("All services shut down. ADA-FS shutdown complete.");
}

bool init_io_tasklet_pool() {
    vector<ABT_xstream> io_streams_tmp(IO_THREADS);
    ABT_pool io_pools_tmp;
//    auto ret = ABT_snoozer_xstream_create(IO_THREADS, &RPC_DATA->io_pools_, RPC_DATA->io_streams_.data());
    auto ret = ABT_snoozer_xstream_create(IO_THREADS, &io_pools_tmp, io_streams_tmp.data());
    if (ret != ABT_SUCCESS) {
        ADAFS_DATA->spdlogger()->error(
                "{}() ABT_snoozer_xstream_create() failed to initialize ABT_pool for I/O operations", __func__);
        return false;
    }
    RPC_DATA->io_streams(io_streams_tmp);
    RPC_DATA->io_pool(io_pools_tmp);

    return true;
}

bool init_ipc_server() {
    auto protocol_port = "na+sm://"s;
    hg_addr_t addr_self;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];


    ADAFS_DATA->spdlogger()->debug("{}() Initializing Margo IPC server...", __func__);
    // Start Margo (this will also initialize Argobots and Mercury internally)
    auto mid = margo_init(protocol_port.c_str(), MARGO_SERVER_MODE, 1, IPC_HANDLER_THREADS);

    if (mid == MARGO_INSTANCE_NULL) {
        ADAFS_DATA->spdlogger()->error("{}() margo_init() failed to initialize the Margo IPC server", __func__);
        return false;
    }
#ifdef MARGODIAG
    margo_diag_start(mid);
#endif
    // Figure out what address this server is listening on (must be freed when finished)
    auto hret = margo_addr_self(mid, &addr_self);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() margo_addr_self() Failed to retrieve server IPC address", __func__);
        margo_finalize(mid);
        return false;
    }
    // Convert the address to a cstring (with \0 terminator).
    hret = margo_addr_to_string(mid, addr_self_cstring, &addr_self_cstring_sz, addr_self);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() margo_addr_to_string() Failed to convert address to cstring", __func__);
        margo_addr_free(mid, addr_self);
        margo_finalize(mid);
        return false;
    }
    margo_addr_free(mid, addr_self);

    ADAFS_DATA->spdlogger()->info("{}() Margo IPC server initialized. Accepting IPCs on PID {}", __func__,
                                  addr_self_cstring);
    // Put context and class into RPC_data object
    RPC_DATA->server_ipc_mid(mid);

    // register RPCs
    register_server_rpcs(mid);

    return true;
}

bool init_rpc_server() {
    auto protocol_port = RPC_PROTOCOL + "://localhost:"s + to_string(RPC_PORT);
    hg_addr_t addr_self;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];
    ADAFS_DATA->spdlogger()->debug("{}() Initializing Margo RPC server...", __func__);
    // Start Margo (this will also initialize Argobots and Mercury internally)
    auto mid = margo_init(protocol_port.c_str(), MARGO_SERVER_MODE, 1, RPC_HANDLER_THREADS);
    if (mid == MARGO_INSTANCE_NULL) {
        ADAFS_DATA->spdlogger()->error("{}() margo_init failed to initialize the Margo RPC server", __func__);
        return false;
    }
#ifdef MARGODIAG
    margo_diag_start(mid);
#endif
    // Figure out what address this server is listening on (must be freed when finished)
    auto hret = margo_addr_self(mid, &addr_self);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() margo_addr_self() Failed to retrieve server RPC address", __func__);
        margo_finalize(mid);
        return false;
    }
    // Convert the address to a cstring (with \0 terminator).
    hret = margo_addr_to_string(mid, addr_self_cstring, &addr_self_cstring_sz, addr_self);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() margo_addr_to_string Failed to convert address to cstring", __func__);
        margo_addr_free(mid, addr_self);
        margo_finalize(mid);
        return false;
    }
    margo_addr_free(mid, addr_self);


    ADAFS_DATA->spdlogger()->info("{}() Margo RPC server initialized. Accepting RPCs on address {}", __func__,
                                  addr_self_cstring);

    // Put context and class into RPC_data object
    RPC_DATA->server_rpc_mid(mid);

    // register RPCs
    register_server_rpcs(mid);

    return true;
}

/**
 * Registers RPC handlers to Margo instance
 * @param hg_class
 */
void register_server_rpcs(margo_instance_id mid) {
    if (RPC_DATA->server_ipc_mid() == mid)
        MARGO_REGISTER(mid, hg_tag::fs_config, ipc_config_in_t, ipc_config_out_t, ipc_srv_fs_config);
    MARGO_REGISTER(mid, hg_tag::minimal, rpc_minimal_in_t, rpc_minimal_out_t, rpc_minimal);
    MARGO_REGISTER(mid, hg_tag::create, rpc_mk_node_in_t, rpc_err_out_t, rpc_srv_mk_node);
    MARGO_REGISTER(mid, hg_tag::access, rpc_access_in_t, rpc_err_out_t, rpc_srv_access);
    MARGO_REGISTER(mid, hg_tag::stat, rpc_path_only_in_t, rpc_stat_out_t, rpc_srv_stat);
    MARGO_REGISTER(mid, hg_tag::remove, rpc_rm_node_in_t, rpc_err_out_t, rpc_srv_rm_node);
    MARGO_REGISTER(mid, hg_tag::update_metadentry, rpc_update_metadentry_in_t, rpc_err_out_t,
                   rpc_srv_update_metadentry);
    MARGO_REGISTER(mid, hg_tag::get_metadentry_size, rpc_path_only_in_t, rpc_get_metadentry_size_out_t,
                   rpc_srv_get_metadentry_size);
    MARGO_REGISTER(mid, hg_tag::update_metadentry_size, rpc_update_metadentry_size_in_t,
                   rpc_update_metadentry_size_out_t, rpc_srv_update_metadentry_size);
    MARGO_REGISTER(mid, hg_tag::write_data, rpc_write_data_in_t, rpc_data_out_t, rpc_srv_write_data);
    MARGO_REGISTER(mid, hg_tag::read_data, rpc_read_data_in_t, rpc_data_out_t, rpc_srv_read_data);
}

/**
 * Returns the path where daemon process writes information for the running clients
 * @return string
 */
string daemon_register_path() {
    return (DAEMON_AUX_PATH + "/daemon_"s + to_string(getpid()) + ".run"s);
}

/**
 * Registers the daemon process to the system.
 * This will create a file with additional information for clients started on the same node.
 * @return
 */
bool register_daemon_proc() {
    auto ret = false;
    auto daemon_aux_path = DAEMON_AUX_PATH;
    if (!bfs::exists(daemon_aux_path) && !bfs::create_directories(daemon_aux_path)) {
        ADAFS_DATA->spdlogger()->error("{}() Unable to create adafs auxiliary directory in {}", __func__,
                                       daemon_aux_path);
        return false;
    }
    ofstream ofs(daemon_register_path().c_str(), ::ofstream::trunc);
    if (ofs) {
        ofs << ADAFS_DATA->mountdir();
        ret = true;
    }
    if (ofs.bad()) {
        perror("Error opening file to register daemon process");
        ADAFS_DATA->spdlogger()->error("{}() Error opening file to register daemon process", __func__);
        return false;
    }
    ofs.close();
    return ret;
}

bool deregister_daemon_proc() {
    return bfs::remove(daemon_register_path());
}

/**
 * Returns the machine's hostname
 * @return
 */
string get_my_hostname(bool short_hostname) {
    char hostname[1024];
    auto ret = gethostname(hostname, 1024);
    if (ret == 0) {
        string hostname_s(hostname);
        if (!short_hostname)
            return hostname_s;
        // get short hostname
        auto pos = hostname_s.find("."s);
        if (pos != std::string::npos)
            hostname_s = hostname_s.substr(0, pos);
        return hostname_s;
    } else
        return ""s;
}

void shutdown_handler(int dummy) {
    shutdown_please.notify_all();
}

int main(int argc, const char* argv[]) {


    //set the spdlogger and initialize it with spdlog
    ADAFS_DATA->spdlogger(spdlog::basic_logger_mt("basic_logger", LOG_DAEMON_PATH));
    // set logger format
    spdlog::set_pattern("[%C-%m-%d %H:%M:%S.%f] %P [%L] %v");
    // flush log when info, warning, error messages are encountered
    ADAFS_DATA->spdlogger()->flush_on(spdlog::level::info);
#if defined(LOG_TRACE)
    spdlog::set_level(spdlog::level::trace);
    ADAFS_DATA->spdlogger()->flush_on(spdlog::level::trace);
#elif defined(LOG_DEBUG)
    spdlog::set_level(spdlog::level::debug);
//    ADAFS_DATA->spdlogger()->flush_on(spdlog::level::debug);
#elif defined(LOG_INFO)
    spdlog::set_level(spdlog::level::info);
//    ADAFS_DATA->spdlogger()->flush_on(spdlog::level::info);
#else
    spdlog::set_level(spdlog::level::off);
#endif

    // Parse input

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "Help message")
            ("mountdir,m", po::value<string>()->required(), "User Fuse mountdir.")
            ("rootdir,r", po::value<string>()->required(), "ADA-FS data directory")
            ("hostfile", po::value<string>(), "Path to the hosts_file for all fs participants")
            ("hosts,h", po::value<string>(), "Comma separated list of hosts_ for all fs participants");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    try {
        po::notify(vm);
    } catch (po::required_option& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    if (vm.count("mountdir")) {
        ADAFS_DATA->mountdir(vm["mountdir"].as<string>());
    }
    if (vm.count("rootdir")) {
        ADAFS_DATA->rootdir(vm["rootdir"].as<string>());
    }
    // parse host parameters
    vector<string> hosts{};
    if (vm.count("hostfile")) {
        auto host_path = vm["hostfile"].as<string>();
        fstream host_file(host_path);
        if (host_file.is_open()) {
            for (string line; getline(host_file, line);) {
                if (line.at(0) != '#') {
                    auto subline = line.substr(0, line.find(' '));
                    hosts.push_back(subline);
                }
            }
        } else {
            cerr << "Hostfile path does not exist. Exiting ..." << endl;
            ADAFS_DATA->spdlogger()->error("{}() Hostfile path does not exist. Exiting ...", __func__);
            assert(host_file.is_open());
        }
    } else if (vm.count("hosts")) {
        // split comma separated host string
        boost::char_separator<char> sep(",");
        boost::tokenizer<boost::char_separator<char>> tok(vm["hosts"].as<string>(), sep);
        for (auto&& s : tok) {
            hosts.push_back(s);
        }
    }
    // convert host parameters into datastructures
    std::map<uint64_t, std::string> hostmap;
    auto hosts_raw = ""s;
    if (!hosts.empty()) {
        auto i = static_cast<uint64_t>(0);
        auto found_hostname = false;
        auto hostname = get_my_hostname(true);
        if (hostname.empty()) {
            cerr << "Unable to read the machine's hostname" << endl;
            ADAFS_DATA->spdlogger()->error("{}() Unable to read the machine's hostname", __func__);
            assert(!hostname.empty());
        }
        for (auto&& host : hosts) {
            hostmap[i] = host;
            hosts_raw += host + ","s;
            if (hostname == host) {
                ADAFS_DATA->host_id(i);
                found_hostname = true;
            }
            i++;
        }
        if (!found_hostname) {
            ADAFS_DATA->spdlogger()->error("{}() Hostname was not found in given parameters. Exiting ...", __func__);
            cerr << "Hostname was not found in given parameters. Exiting ..." << endl;
            assert(found_hostname);
        }
        hosts_raw = hosts_raw.substr(0, hosts_raw.size() - 1);
    } else {
        // single node mode
        ADAFS_DATA->spdlogger()->info("{}() Single node mode set to self", __func__);
        auto hostname = get_my_hostname(false);
        hostmap[0] = hostname;
        hosts_raw = hostname;
        ADAFS_DATA->host_id(0);
    }
    ADAFS_DATA->hosts(hostmap);
    ADAFS_DATA->host_size(hostmap.size());
    ADAFS_DATA->rpc_port(fmt::FormatInt(RPC_PORT).str());
    ADAFS_DATA->hosts_raw(hosts_raw);



    //set all paths
    ADAFS_DATA->inode_path(ADAFS_DATA->rootdir() + "/meta/inodes"s); // XXX prob not needed anymore
    ADAFS_DATA->dentry_path(ADAFS_DATA->rootdir() + "/meta/dentries"s); // XXX prob not needed anymore
    ADAFS_DATA->chunk_path(ADAFS_DATA->rootdir() + "/data/chunks"s);
    ADAFS_DATA->mgmt_path(ADAFS_DATA->rootdir() + "/mgmt"s);

    ADAFS_DATA->spdlogger()->info("{}() Initializing environment. Hold on ...", __func__);

    // Make sure directory structure exists
    bfs::create_directories(ADAFS_DATA->dentry_path());
    bfs::create_directories(ADAFS_DATA->inode_path());
    bfs::create_directories(ADAFS_DATA->chunk_path());
    bfs::create_directories(ADAFS_DATA->mgmt_path());
    // Create mountdir. We use this dir to get some information on the underlying fs with statfs in adafs_statfs
    bfs::create_directories(ADAFS_DATA->mountdir());

    if (init_environment()) {
        signal(SIGINT, shutdown_handler);
        signal(SIGTERM, shutdown_handler);
        signal(SIGKILL, shutdown_handler);

        unique_lock<mutex> lk(mtx);
        // Wait for shutdown signal to initiate shutdown protocols
        shutdown_please.wait(lk);
        ADAFS_DATA->spdlogger()->info("{}() Shutting done signal encountered. Shutting down ...", __func__);
    } else {
        ADAFS_DATA->spdlogger()->info("{}() Starting up daemon environment failed. Shutting down ...", __func__);
    }

    destroy_enviroment();

    return 0;

}