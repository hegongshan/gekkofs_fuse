
#include "main.hpp"
#include <future>
#include "classes/metadata.hpp"
#include "adafs_ops/mdata_ops.hpp"
#include "adafs_ops/dentry_ops.hpp"
#include "fuse_ops.hpp"
#include "rpc/rpc_util.hpp"


static struct fuse_lowlevel_ops adafs_ops;

using namespace std;
namespace po = boost::program_options;

struct tmp_fuse_usr {
    // Map host nr to host
    std::map<uint64_t, std::string> hosts;
    std::string hostfile;
    uint64_t host_nr;
};

/**
 * Initializes the rpc environment: Mercury with Argobots = Margo
 * This must be run in a dedicated thread!
 */
void init_rpc_env(promise<bool> rpc_promise) {
    auto ret = init_argobots();
    if (!ret) {
        rpc_promise.set_value(false);
        return;
    }
    ret = init_rpc_server();
    if (!ret) {
        rpc_promise.set_value(false);
        return;
    }
    auto mid = RPC_DATA->server_mid();
    ret = init_rpc_client();
    if (!ret) {
        rpc_promise.set_value(false);
        return;
    }
    rpc_promise.set_value(true);
    margo_wait_for_finalize(
            mid); // XXX this consumes 1 logical core. Should put a conditional variable here and wait until shutdown.
}

/**
 * Initialize filesystem
 *
 * This function is called when libfuse establishes
 * communication with the FUSE kernel module. The file system
 * should use this module to inspect and/or modify the
 * connection parameters provided in the `conn` structure.
 *
 * Note that some parameters may be overwritten by options
 * passed to fuse_session_new() which take precedence over the
 * values set in this handler.
 *
 * There's no reply to this function
 *
 * @param userdata the user data passed to fuse_session_new()
 */
void adafs_ll_init(void* pdata, struct fuse_conn_info* conn) {
    ADAFS_DATA->spdlogger()->info("adafs_ll_init() enter"s);

    // parse additional arguments to adafs
    auto fuse_data = static_cast<tmp_fuse_usr*>(pdata);
    ADAFS_DATA->hosts(fuse_data->hosts);
    ADAFS_DATA->host_id(fuse_data->host_nr);
    ADAFS_DATA->host_size(fuse_data->hosts.size());
    ADAFS_DATA->rpc_port(fmt::FormatInt(RPCPORT).str());

    // Make sure directory structure exists
    bfs::create_directories(ADAFS_DATA->dentry_path());
    bfs::create_directories(ADAFS_DATA->inode_path());
    bfs::create_directories(ADAFS_DATA->chunk_path());
    bfs::create_directories(ADAFS_DATA->mgmt_path());

    // Initialize rocksdb
    auto err = init_rocksdb();
    assert(err);
    // Starting RPC environment
    promise<bool> rpc_promise;
    future<bool> rpc_future = rpc_promise.get_future();
    thread t1(init_rpc_env, move(rpc_promise));
    rpc_future.wait(); // wait for RPC environment to be initialized
    assert(rpc_future.get()); // get potential error during RPC init and exit if future holds false
    ADAFS_DATA->spdlogger()->info("RPC environment successfully started");
    t1.detach(); // detach rpc thread for independent operation. This is mandatory for the Margo framework!

    // Check if fs already has some data and read the inode count
    if (bfs::exists(ADAFS_DATA->mgmt_path() + "/inode_count"))
        Util::read_inode_cnt();
    else
        Util::init_inode_no();

    //Init file system configuration
    ADAFS_DATA->blocksize(4096);

    // Init unordered_map for caching metadata that was already looked up XXX use later
    ADAFS_DATA->hashmap(unordered_map<string, string>()); //unused
    ADAFS_DATA->hashf(hash<string>());

    auto md = make_shared<Metadata>();

    ADAFS_DATA->spdlogger()->info("Checking root metadata...");
    // Check that root metadata exists. If not initialize it
    if (get_metadata(*md, ADAFS_ROOT_INODE) == ENOENT) {
        ADAFS_DATA->spdlogger()->info("Root metadata not found. Initializing..."s);
        md->init_ACM_time();
        md->mode(S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO); // change_access 777
        md->size(4096); // XXX just visual. size computation of directory should be done properly at some point
        // This is because fuse is mounted through root although it was triggered by the user
        md->uid(0); // hardcoded root XXX
        md->gid(0); // hardcoded root XXX
        md->inode_no(ADAFS_ROOT_INODE);
        ADAFS_DATA->spdlogger()->info("Writing / metadata to disk..."s);
        write_all_metadata(*md);
        ADAFS_DATA->spdlogger()->info("Initializing dentry for /"s);
        init_dentry_dir(ADAFS_ROOT_INODE);
        ADAFS_DATA->spdlogger()->info("Creating Metadata object"s);
    }
#ifdef LOG_INFO
    else
        ADAFS_DATA->spdlogger()->info("Root metadata found"s);
#endif

}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit
 *
 * There's no reply to this function
 *
 * @param userdata the user data passed to fuse_session_new()
 */
void adafs_ll_destroy(void* pdata) {
    ADAFS_DATA->spdlogger()->info("Shutting down..."s);
    // Shutting down RPC environment XXX LATER
//    margo_finalize(RPC_DATA->client_mid());
//    ADAFS_DATA->spdlogger()->info("Margo client finalized");
//    margo_finalize(RPC_DATA->server_mid());
//    ADAFS_DATA->spdlogger()->info("Margo server finalized");
//    destroy_argobots();
//    ADAFS_DATA->spdlogger()->info("Argobots shut down"s);
//    destroy_rpc_client();
//    ADAFS_DATA->spdlogger()->info("Client shut down"s);
//    destroy_rpc_server();
//    ADAFS_DATA->spdlogger()->info("Server shut down"s);
    Util::write_inode_cnt();
}

static void init_adafs_ops(fuse_lowlevel_ops* ops) {
    // file
    ops->getattr = adafs_ll_getattr;
    ops->setattr = adafs_ll_setattr;
    ops->create = adafs_ll_create;
    ops->mknod = adafs_ll_mknod;
    ops->unlink = adafs_ll_unlink;
    ops->open = adafs_ll_open;
    ops->release = adafs_ll_release;

    // directory
    ops->lookup = adafs_ll_lookup;
    ops->opendir = adafs_ll_opendir;
    ops->readdir = adafs_ll_readdir;
    ops->mkdir = adafs_ll_mkdir;
    ops->rmdir = adafs_ll_rmdir;
    ops->releasedir = adafs_ll_releasedir;

    // I/O

    // sync
    ops->flush = adafs_ll_flush;

    // permission
    ops->access = adafs_ll_access;

    ops->init = adafs_ll_init;
    ops->destroy = adafs_ll_destroy;
}

void err_cleanup1(fuse_cmdline_opts opts, fuse_args& args) {
    free(opts.mountpoint);
    fuse_opt_free_args(&args);
    cout << "# Resources released" << endl;
}

void err_cleanup2(fuse_session& se) {
    fuse_session_destroy(&se);
    cout << "# Fuse session destroyed" << endl;
}

void err_cleanup3(fuse_session& se) {
    fuse_remove_signal_handlers(&se);
    cout << "# Signal handlers removed" << endl;
}

/**
 * First some adafs configurations, e.g., userdata in fuse is set, then fuse is initialized with the remaining argv
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char* argv[]) {

    //Initialize the mapping of Fuse functions
    init_adafs_ops(&adafs_ops);

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
    auto fuse_argc = 1;
    vector<string> fuse_argv;
    fuse_argv.push_back(move(argv[0]));
    auto fuse_struct = make_unique<tmp_fuse_usr>();

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "Help message")
            ("foreground,f", "Run Fuse instance in foreground. (Fuse parameter)")
            ("mountdir,m", po::value<string>(), "User Fuse mountdir. (Fuse parameter)")
            ("rootdir,r", po::value<string>(), "ADA-FS data directory")
            ("hostsfile", po::value<string>(), "Path to the hosts_ file for all fs participants")
            ("hosts,h", po::value<string>(), "Comma separated list of hosts_ for all fs participants");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    if (vm.count("foreground")) {
        fuse_argc++;
        fuse_argv.push_back("-f"s);
    }
    if (vm.count("mountdir")) {
        fuse_argc++;
        fuse_argv.push_back(vm["mountdir"].as<string>());
    }
    if (vm.count("rootdir")) {
        ADAFS_DATA->rootdir(vm["rootdir"].as<string>());
    }

    // TODO Hostfile parsing here...
    if (vm.count("hosts")) {
        auto hosts = vm["hosts"].as<string>();
        uint64_t i = 0;
        auto found_hostname = false;
        auto hostname = Util::get_my_hostname();
        if (hostname.size() == 0) {
            cerr << "Unable to read the machine's hostname" << endl;
            assert(hostname.size() != 0);
        }
        // split comma separated host string
        boost::tokenizer<> tok(hosts);
        for (auto&& s : tok) {
            fuse_struct->hosts[i] = s;
            if (hostname == s) {
                fuse_struct->host_nr = i;
                found_hostname = true;
            }
            i++;
        }
        if (!found_hostname) {
            cerr << "Hostname was not found in given parameters. Exiting ..." << endl;
            assert(found_hostname);
        }
    }

    // convert fuse_argv into char* []
    char* fuse_argv_c[10] = {0};
    for (unsigned int i = 0; i < fuse_argv.size(); ++i) {
        char* tmp_c = new char[fuse_argv[i].size() + 1];
        std::strcpy(tmp_c, fuse_argv[i].c_str());
        fuse_argv_c[i] = tmp_c;
    }

    //set all paths
    ADAFS_DATA->inode_path(ADAFS_DATA->rootdir() + "/meta/inodes"s);
    ADAFS_DATA->dentry_path(ADAFS_DATA->rootdir() + "/meta/dentries"s);
    ADAFS_DATA->chunk_path(ADAFS_DATA->rootdir() + "/data/chunks"s);
    ADAFS_DATA->mgmt_path(ADAFS_DATA->rootdir() + "/mgmt"s);

    // Fuse stuff starts here in C style... ########################################################################
    struct fuse_args args = FUSE_ARGS_INIT(fuse_argc, fuse_argv_c);
    struct fuse_session* se;
    struct fuse_cmdline_opts opts;
    int err = -1;

    if (fuse_parse_cmdline(&args, &opts) != 0)
        return 1;
    if (opts.show_help) {
        cout << "usage: " << argv[0] << "[option] <mountpoint>\n\n" << endl;
        fuse_cmdline_help();
        fuse_lowlevel_help();
        err_cleanup1(opts, args);
        return 0;
    } else if (opts.show_version) {
        cout << "FUSE library version " << fuse_pkgversion() << "\n" << endl;
        fuse_lowlevel_version();
        err_cleanup1(opts, args);
        return 0;
    }

    // creating a low level session
    se = fuse_session_new(&args, &adafs_ops, sizeof(adafs_ops), fuse_struct.get());

    if (se == NULL) {
        err_cleanup1(opts, args);
        return 0;
    }
    cout << "# Fuse session created" << endl;
    // setup the signal handlers so that fuse exits the session properly
    if (fuse_set_signal_handlers(se) != 0) {
        err_cleanup2(*se);
        err_cleanup1(opts, args);
        return 0;
    }
    cout << "# Signal handlers set" << endl;
    // mount the file system at the mountpoint
    if (fuse_session_mount(se, opts.mountpoint) != 0) {
        err_cleanup3(*se);
        err_cleanup2(*se);
        err_cleanup1(opts, args);
        return 0;
    }
    cout << "# Fuse file system mounted at \"" << opts.mountpoint << "\"" << endl;
    fuse_daemonize(opts.foreground);
    cout << "# Fuse daemonized - About to dive into event loop" << endl;

    // Block until ctrl+c or fusermount -u
    if (opts.singlethread)
        err = fuse_session_loop(se);
    else
        err = fuse_session_loop_mt(se, opts.clone_fd);

    cout << "\n# Interrupt detected \n# Destroying file system..." << endl;

    // Cleanup
    fuse_session_unmount(se);
    cout << "# Fuse file system unmounted" << endl;
    err_cleanup3(*se);
    err_cleanup2(*se);
    err_cleanup1(opts, args);

    return err ? 1 : 0;
}
