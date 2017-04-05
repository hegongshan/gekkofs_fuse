#include "main.h"
#include "classes/metadata.h"
#include "adafs_ops/metadata_ops.h"
#include "adafs_ops/dentry_ops.h"
#include "fuse_ops.h"

static struct adafs_data *adafs_data(fuse_req_t req) {
    return (struct adafs_data *) fuse_req_userdata(req);
}

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


int main(int argc, char* argv[]) {
    //Initialize the mapping of Fuse functions
//    init_adafs_ops(&adafs_ops);

    // create the private data struct
    auto a_data = make_shared<adafs_data>();
    //set the logger and initialize it with spdlog
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
    //print version
    cout << "Fuse library version: "s + to_string(FUSE_MAJOR_VERSION) + to_string(FUSE_MINOR_VERSION) << endl;
    //init fuse and give the private data struct for further reference.
    cout << "initializing fuse..." << endl;
//    auto err = fuse_main(argc, argv, &adafs_ops, a_data.get());
    cout << "about to close fuse" << endl;

    return 0;
}
