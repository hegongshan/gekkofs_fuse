#include "main.h"
#include "classes/metadata.h"
#include "adafs_ops/metadata_ops.h"
#include "adafs_ops/dentry_ops.h"
#include "fuse_ops.h"

static struct fuse_operations adafs_ops;

using namespace std;

void* adafs_init(struct fuse_conn_info* conn, struct fuse_config* cfg) {
    ADAFS_DATA->logger->debug("Fuse init() enter"s);
    // Make sure directory structure exists
    bfs::create_directories(ADAFS_DATA->dentry_path);
    bfs::create_directories(ADAFS_DATA->inode_path);
    bfs::create_directories(ADAFS_DATA->chunk_path);
    bfs::create_directories(ADAFS_DATA->mgmt_path);

    // Check if fs already has some data and read the inode count
    if (bfs::exists(ADAFS_DATA->mgmt_path + "/inode_count"))
        util::read_inode_cnt();
    else
        util::init_inode_no();

    // XXX Dunno if this is of any use
    cfg->readdir_ino = 1;

    //Init file system configuration
    ADAFS_DATA->blocksize = 4096;

    // Init unordered_map for caching metadata that was already looked up XXX use later
    ADAFS_DATA->hashmap = unordered_map<string, string>();
    ADAFS_DATA->hashf = hash<string>();

    auto md = make_shared<Metadata>();

    // Check that root metadata exists. If not initialize it
    if (get_metadata(*md, "/"s) == -ENOENT) {
        ADAFS_DATA->logger->debug("Root metadata not found. Initializing..."s);
        md->init_ACM_time();
        md->mode(S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO); // change_access 777
        md->size(4096); // XXX just visual. size computation of directory should be done properly at some point
//        md->mode(S_IFDIR | 0755);
        // XXX The gid and uid is the root user for some reason. Should be the user that mounted fuse.
        // This is because fuse is mounted through root although it was triggered by the user
        md->uid(fuse_get_context()->uid);
        md->gid(fuse_get_context()->gid);
        md->inode_no(ADAFS_ROOT_INODE);
        ADAFS_DATA->logger->debug("Writing / metadata to disk..."s);
        write_all_metadata(*md, ADAFS_DATA->hashf("/"s));
        ADAFS_DATA->logger->debug("Initializing dentry for /"s);
        init_dentry_dir(ADAFS_DATA->hashf("/"s));
        ADAFS_DATA->logger->debug("Creating Metadata object"s);
    }
#ifdef LOG_DEBUG
    else
        ADAFS_DATA->logger->debug("Metadata object exists"s);
#endif


    return ADAFS_DATA;
}

void adafs_destroy(void* adafs_data) {
    util::write_inode_cnt();
    delete ADAFS_DATA;
}


static void init_adafs_ops(fuse_operations* ops) {
    // file
    ops->getattr = adafs_getattr;
    ops->mknod = adafs_mknod;
    ops->create = adafs_create;
    ops->open = adafs_open;
    ops->unlink = adafs_unlink;
    ops->utimens = adafs_utimens;
    ops->release = adafs_release;

    // directory
    ops->opendir = adafs_opendir;
    ops->readdir = adafs_readdir;
    ops->releasedir = adafs_releasedir;
    ops->mkdir = adafs_mkdir;
    ops->rmdir = adafs_rmdir;

    // I/O
    ops->read = adafs_read;
    ops->write = adafs_write;
    ops->truncate = adafs_truncate;

    ops->flush = adafs_flush;

    // permission
    ops->access = adafs_access;
    ops->chmod = adafs_chmod;
    ops->chown = adafs_chown;

    ops->init = adafs_init;
    ops->destroy = adafs_destroy;
}


int main(int argc, char* argv[]) {
    //Initialize the mapping of Fuse functions
    init_adafs_ops(&adafs_ops);

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
    return fuse_main(argc, argv, &adafs_ops, a_data.get());
}
