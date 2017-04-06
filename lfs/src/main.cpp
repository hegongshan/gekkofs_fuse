#include "main.h"
#include "classes/metadata.h"
#include "adafs_ops/metadata_ops.h"
#include "adafs_ops/dentry_ops.h"
#include "fuse_ops.h"

//static struct adafs_data *adafs_data(fuse_req_t req) {
//    return (struct adafs_data *) fuse_req_userdata(req);
//}

static struct fuse_lowlevel_ops adafs_ops;

using namespace std;

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
void adafs_init(void* adafs_data, struct fuse_conn_info* conn) {

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
void adafs_destroy(void* adafs_data) {

}

static void init_adafs_ops(fuse_lowlevel_ops* ops) {
    // file

    // directory

    // I/O

    // permission

    ops->init = adafs_init;
    ops->destroy = adafs_destroy;
}

void err_out1(fuse_cmdline_opts opts, fuse_args& args) {
    free(opts.mountpoint);
    fuse_opt_free_args(&args);
    cout << "# Resources released" << endl;
}

void err_out2(fuse_session& se) {
    fuse_session_destroy(&se);
    cout << "# Fuse session destroyed" << endl;
}

void err_out3(fuse_session& se) {
    fuse_remove_signal_handlers(&se);
    cout << "# Signal handlers removed" << endl;
}


/**
 * First some adafs configurations, e.g., userdata in fuse is set, then fuse is initialized with the remaining argv
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[]) {

    //Initialize the mapping of Fuse functions
    init_adafs_ops(&adafs_ops);

    // create the private data struct
    auto a_data = make_shared<adafs_data>();
//    //set the logger and initialize it with spdlog
    a_data->logger = spdlog::basic_logger_mt("basic_logger", "adafs.log");
#if defined(LOG_DEBUG)
    spdlog::set_level(spdlog::level::debug);
    a_data->logger->flush_on(spdlog::level::debug);
#elif defined(LOG_INFO)
    spdlog::set_level(spdlog::level::info);
    a_data->logger->flush_on(spdlog::level::info);
#else
    spdlog::set_level(spdlog::level::off);
#endif
    //extract the rootdir from argv and put it into rootdir of adafs_data
    a_data->rootdir = string(realpath(argv[argc - 2], NULL));
    argv[argc - 2] = argv[argc - 1];
    argv[argc - 1] = NULL;
    argc--;
    //set all paths
    a_data->inode_path = a_data->rootdir + "/meta/inodes"s;
    a_data->dentry_path = a_data->rootdir + "/meta/dentries"s;
    a_data->chunk_path = a_data->rootdir + "/data/chunks"s;
    a_data->mgmt_path = a_data->rootdir + "/mgmt"s;

    // Fuse stuff starts here ###
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    struct fuse_session* se;
    struct fuse_cmdline_opts opts;
    int err = -1;

    if (fuse_parse_cmdline(&args, &opts) != 0)
        return 1;
    if (opts.show_help) {
        cout << "usage: " << argv[0] << "[option] <mountpoint>\n\n" << endl;
        fuse_cmdline_help();
        fuse_lowlevel_help();
        err_out1(opts, args);
        return 0;
    } else if (opts.show_version) {
        cout << "FUSE library version " << fuse_pkgversion() << "\n" << endl;
        fuse_lowlevel_version();
        err_out1(opts, args);
        return 0;
    }

    // creating a low level session
    se = fuse_session_new(&args, &adafs_ops, sizeof(adafs_ops), NULL);

    if (se == NULL) {
        err_out1(opts, args);
        return 0;
    }
    cout << "# Fuse session created" << endl;
    // setup the signal handlers so that fuse exits the session properly
    if (fuse_set_signal_handlers(se) != 0) {
        err_out2(*se);
        err_out1(opts, args);
        return 0;
    }
    cout << "# Signal handlers set" << endl;
    if (fuse_session_mount(se, opts.mountpoint) != 0) {
        err_out3(*se);
        err_out2(*se);
        err_out1(opts, args);
        return 0;
    }
    cout << "# Fuse file system mounted at \"" << opts.mountpoint << "\"" << endl;
    fuse_daemonize(opts.foreground);
    cout << "# Fuse demonized - About to dive into event loop" << endl;

    // Block until ctrl+c or fusermount -u
    if (opts.singlethread)
        err = fuse_session_loop(se);
    else
        err = fuse_session_loop_mt(se, opts.clone_fd);

    cout << "\n# Interrupt detected \n# Destroying file system..." << endl;

    fuse_session_unmount(se);
    cout << "# Fuse file system unmounted" << endl;
    err_out3(*se);
    err_out2(*se);
    err_out1(opts, args);

    return err ? 1 : 0;
}
