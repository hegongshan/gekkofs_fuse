
#include <fuse3/fuse_main.hpp>
#include <fuse3/fuse_ops.hpp>
#include <fuse3/fuse_util.hpp>

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
 * Initializes the Argobots environment
 * @return
 */
bool init_fuse_argobots() {
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

void register_client_fuse_ipcs() {
//    minimal_id = MERCURY_REGISTER(mercury_hg_class_, "rpc_minimal", rpc_minimal_in_tt, rpc_minimal_out_tt, nullptr);
//    ipc_open_id = MERCURY_REGISTER(mercury_hg_class_, "ipc_srv_open", ipc_open_in_t, ipc_res_out_t, nullptr);
//    ipc_stat_id = MERCURY_REGISTER(mercury_hg_class_, "ipc_srv_stat", ipc_stat_in_t, ipc_stat_out_t, nullptr);
//    ipc_unlink_id = MERCURY_REGISTER(mercury_hg_class_, "ipc_srv_unlink", ipc_unlink_in_t, ipc_res_out_t, nullptr);
//    ipc_config_id = MERCURY_REGISTER(mercury_hg_class_, "ipc_srv_fs_config", ipc_config_in_t, ipc_config_out_t,
//                                     nullptr);
}

bool init_ipc_fuse_client() {
    auto protocol_port = RPC_PROTOCOL+s;
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

/**
 * Initializes the rpc environment: Mercury with Argobots = Margo
 * This must be run in a dedicated thread!
 */
void init_rpc_env(promise<bool> rpc_promise) {
    auto ret = init_fuse_argobots();
    if (!ret) {
        rpc_promise.set_value(false);
        return;
    }
    auto mid = RPC_DATA->server_mid();
    ret = init_ipc_fuse_client();
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
    ops->write = adafs_ll_write;
    ops->read = adafs_ll_read;

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
 * Starts up fuse with ADA-FS
 * @param argc
 * @param argv
 * @return
 */
int fuse_main(int fuse_argc, char* fuse_argv_c[]) {

    // Fuse stuff starts here in C style... ########################################################################
    struct fuse_args args = FUSE_ARGS_INIT(fuse_argc, fuse_argv_c);
    struct fuse_session* se;
    struct fuse_cmdline_opts opts;
    int err = -1;

    if (fuse_parse_cmdline(&args, &opts) != 0)
        return 1;
    if (opts.show_help) {
        cout << "usage: " << "[option] <mountpoint>\n\n" << endl; // XXX What does that do again?
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
    se = fuse_session_new(&args, &adafs_ops, sizeof(adafs_ops), nullptr);

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