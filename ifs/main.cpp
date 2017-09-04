#include "main.hpp"

#include "daemon/adafs_daemon.hpp"


using namespace std;
namespace po = boost::program_options;


int main(int argc, const char* argv[]) {


//    //set the spdlogger and initialize it with spdlog
    ADAFS_DATA->spdlogger(spdlog::basic_logger_mt("basic_logger", "adafs.log"));
#if defined(LOG_TRACE)
    spdlog::set_level(spdlog::level::trace);
    ADAFS_DATA->spdlogger()->flush_on(spdlog::level::trace);
#elif defined(LOG_DEBUG)
    spdlog::set_level(spdlog::level::debug);
    ADAFS_DATA->spdlogger()->flush_on(spdlog::level::debug);
#elif defined(LOG_INFO)
    spdlog::set_level(spdlog::level::info);
    ADAFS_DATA->spdlogger()->flush_on(spdlog::level::info);
#else
    spdlog::set_level(spdlog::level::off);
#endif

    // Parse input
    vector<string> fuse_argv;
    fuse_argv.push_back(move(argv[0]));

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "Help message")
            ("mountdir,m", po::value<string>(), "User Fuse mountdir.")
            ("rootdir,r", po::value<string>(), "ADA-FS data directory")
            ("hostsfile", po::value<string>(), "Path to the hosts_file for all fs participants")
            ("hosts,h", po::value<string>(), "Comma separated list of hosts_ for all fs participants");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    if (vm.count("mountdir")) {
        fuse_argv.push_back(vm["mountdir"].as<string>());
    }
    if (vm.count("rootdir")) {
        ADAFS_DATA->rootdir(vm["rootdir"].as<string>());
    }

//    // TODO Hostfile parsing here...
//    if (vm.count("hosts")) {
//        auto hosts = vm["hosts"].as<string>();
//        uint64_t i = 0;
//        auto found_hostname = false;
//        auto hostname = Util::get_my_hostname();
//        if (hostname.size() == 0) {
//            cerr << "Unable to read the machine's hostname" << endl;
//            assert(hostname.size() != 0);
//        }
//        // split comma separated host string
//        boost::char_separator<char> sep(",");
//        boost::tokenizer<boost::char_separator<char>> tok(hosts, sep);
//        for (auto&& s : tok) {
//            fuse_struct->hosts[i] = s;
//            if (hostname == s) {
//                fuse_struct->host_nr = i;
//                found_hostname = true;
//            }
//            i++;
//        }
//        if (!found_hostname) {
//            cerr << "Hostname was not found in given parameters. Exiting ..." << endl;
//            assert(found_hostname);
//        }
//    }

    //set all paths
    ADAFS_DATA->inode_path(ADAFS_DATA->rootdir() + "/meta/inodes"s);
    ADAFS_DATA->dentry_path(ADAFS_DATA->rootdir() + "/meta/dentries"s);
    ADAFS_DATA->chunk_path(ADAFS_DATA->rootdir() + "/data/chunks"s);
    ADAFS_DATA->mgmt_path(ADAFS_DATA->rootdir() + "/mgmt"s);

    ADAFS_DATA->spdlogger()->info("adafs_ll_init() enter"s);

    // Make sure directory structure exists
    bfs::create_directories(ADAFS_DATA->dentry_path());
    bfs::create_directories(ADAFS_DATA->inode_path());
    bfs::create_directories(ADAFS_DATA->chunk_path());
    bfs::create_directories(ADAFS_DATA->mgmt_path());

    init_environment();

    run_daemon(); // blocks here until application loop is exited TODO don't know yet how it'll be closed :D



    destroy_enviroment();

    return 0;

}