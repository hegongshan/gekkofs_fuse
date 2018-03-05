#include "main.hpp"

#include "daemon/adafs_daemon.hpp"

#include <csignal>
#include <condition_variable>


using namespace std;
namespace po = boost::program_options;

static condition_variable shutdown_please;
static mutex mtx;

void shutdown_handler(int dummy) {
    shutdown_please.notify_all();
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

    try{
        po::notify(vm);
    }catch(po::required_option& e){
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