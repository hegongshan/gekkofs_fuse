#include "main.h"
#include "classes/metadata.h"
#include "adafs_ops/metadata_ops.h"
#include "adafs_ops/dentry_ops.h"
#include "fuse_ops.h"

static struct fuse_lowlevel_ops adafs_ops;

using namespace std;

std::shared_ptr<Metadata> md;

/**
 * Get file attributes.
 *
 * If writeback caching is enabled, the kernel may have a
 * better idea of a file's length than the FUSE file system
 * (eg if there has been a write that extended the file size,
 * but that has not yet been passed to the filesystem.n
 *
 * In this case, the st_size value provided by the file system
 * will be ignored.
 *
 * Valid replies:
 *   fuse_reply_attr
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param fi for future use, currently always NULL
 */
void adafs_ll_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    ADAFS_DATA(req)->logger->debug("##### FUSE FUNC ###### adafs_getattr() enter: inode {}", ino);

    auto attr = make_unique<struct stat>();

    if (ino == 1) {
        attr->st_ino = md->inode_no();
        attr->st_mode = md->mode();
        attr->st_nlink = md->link_count();
        attr->st_uid = md->uid();
        attr->st_gid = md->gid();
        attr->st_size = md->size();
        attr->st_blksize = ADAFS_DATA(req)->blocksize;
        attr->st_blocks = md->blocks();
        attr->st_atim.tv_sec = md->atime();
        attr->st_mtim.tv_sec = md->mtime();
        attr->st_ctim.tv_sec = md->ctime();

        fuse_reply_attr(req, attr.get(), 1.0);
    } else {
        fuse_reply_err(req, ENOENT);
    }
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
void adafs_ll_init(void* adata, struct fuse_conn_info* conn) {
    auto adafs_data = (struct adafs_data*) adata;

    adafs_data->logger->debug("##### FUSE FUNC ###### adafs_ll_init() enter"s);
    // Make sure directory structure exists
    bfs::create_directories(adafs_data->dentry_path);
    bfs::create_directories(adafs_data->inode_path);
    bfs::create_directories(adafs_data->chunk_path);
    bfs::create_directories(adafs_data->mgmt_path);

    // Check if fs already has some data and read the inode count
    if (bfs::exists(adafs_data->mgmt_path + "/inode_count"))
        util::read_inode_cnt(*adafs_data);
    else
        util::init_inode_no(*adafs_data);

    //Init file system configuration
    adafs_data->blocksize = 4096;

    // Init unordered_map for caching metadata that was already looked up XXX use later
    adafs_data->hashmap = unordered_map<string, string>();
    adafs_data->hashf = hash<string>();

    md = make_shared<Metadata>();
//    auto md = make_shared<Metadata>();

    // Check that root metadata exists. If not initialize it XXX when get_metadata is back put it back in
//    if (get_metadata(*md, "/"s) == -ENOENT) {
    adafs_data->logger->debug("Root metadata not found. Initializing..."s);
    md->init_ACM_time();
    md->mode(S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO); // change_access 777
    md->size(4096); // XXX just visual. size computation of directory should be done properly at some point
//        md->mode(S_IFDIR | 0755);
    // This is because fuse is mounted through root although it was triggered by the user
    md->uid(0); // hardcoded root XXX
    md->gid(0); // hardcoded root XXX
    md->inode_no(ADAFS_ROOT_INODE);
    adafs_data->logger->debug("Writing / metadata to disk..."s);
//        write_all_metadata(*md, adafs_data->hashf("/"s));
    adafs_data->logger->debug("Initializing dentry for /"s);
//        init_dentry_dir(adafs_data->hashf("/"s));
    adafs_data->logger->debug("Creating Metadata object"s);
//    }
//#ifdef LOG_DEBUG
//    else
//        adafs_data->logger->debug("Metadata object exists"s);
//#endif

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
void adafs_ll_destroy(void* adata) {
    auto adafs_data = (struct adafs_data*) adata;
    util::write_inode_cnt(*adafs_data);
}

static void init_adafs_ops(fuse_lowlevel_ops* ops) {
    // file
    ops->getattr = adafs_ll_getattr;
    // directory

    // I/O

    // permission

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

    // Fuse stuff starts here in C style... ########################################################################
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
        err_cleanup1(opts, args);
        return 0;
    } else if (opts.show_version) {
        cout << "FUSE library version " << fuse_pkgversion() << "\n" << endl;
        fuse_lowlevel_version();
        err_cleanup1(opts, args);
        return 0;
    }

    // creating a low level session
    se = fuse_session_new(&args, &adafs_ops, sizeof(adafs_ops), a_data.get());

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
