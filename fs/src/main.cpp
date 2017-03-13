#include "main.h"
#include "metadata.h"
#include "metadata_ops.h"
#include "fuse_ops.h"

static struct fuse_operations adafs_ops;

using namespace std;

//std::shared_ptr<Metadata> md;

int adafs_getattr(const char *path, struct stat *attr){
    auto fpath = util::adafs_fullpath("meta/inodes"s);
    auto md = make_shared<Metadata>();
    md->mode(S_IFDIR | 0755);
    md->inode_no(1);
    read_all_metadata(*md, 1, fpath);
    if (strcmp(path, "/") == 0) {
        attr->st_ino = md->inode_no();
        attr->st_mode = md->mode();
        attr->st_nlink = md->link_count();
        attr->st_uid = md->uid();
        attr->st_gid = md->gid();
        attr->st_size = md->size();
        attr->st_blksize = ADAFS_DATA->blocksize;
        attr->st_blocks = md->blocks();
        attr->st_atim.tv_sec = md->atime();
        attr->st_mtim.tv_sec = md->mtime();
        attr->st_ctim.tv_sec = md->ctime();
        return 0;
    }

    if (strcmp(path, "/file") == 0) {
        attr->st_mode = S_IFREG | 0755;
        attr->st_nlink = 1;
        attr->st_size = strlen("blubb");
        return 0;
    }
    return -ENOENT;
}

void *adafs_init(struct fuse_conn_info *conn) {

//    ADAFS_DATA->logger->info("init function"s);
//    ADAFS_DATA->logger->info("uid_: {}", fuse_get_context()->uid);
//    ADAFS_DATA->logger->info("gid_: {}", fuse_get_context()->gid);
//    ADAFS_DATA->logger->info("pid: {0:d}", fuse_get_context()->pid);
//    ADAFS_DATA->logger->info("rootdir: {}", ((struct adafs_data*)fuse_get_context()->private_data)->rootdir);
    //Initialize directory structure for metadata.
    boost::filesystem::create_directories(ADAFS_DATA->rootdir + "/meta/dentries"s);
    boost::filesystem::create_directories(ADAFS_DATA->rootdir + "/meta/inodes"s);
    boost::filesystem::create_directories(ADAFS_DATA->rootdir + "/data/chunks"s);
    //Init file system configuration
    ADAFS_DATA->blocksize = 4096;


    //Init metadata
//    if (get_metadata("/") == -ENOENT) {
//        md = make_shared<Metadata>(S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO);
//        auto md = make_shared<Metadata>(S_IFDIR | 0755);
//    }
//    auto s = util::adafs_fullpath("meta/inodes");
//

//    write_all_metadata(*md, s);

    ADAFS_DATA->logger->info("Survived creating Metadata object"s);
    ADAFS_DATA->logger->flush();

    return ADAFS_DATA;
}

void adafs_destroy(void *adafs_data) {
    delete ADAFS_DATA;
}


static void init_adafs_ops(fuse_operations *ops) {
    ops->getattr = adafs_getattr;

    ops->init = adafs_init;
    ops->destroy = adafs_destroy;
}



int main(int argc, char *argv[]) {
    //Initialize the mapping of Fuse functions
    init_adafs_ops(&adafs_ops);

    // create the adafs_data object (struct)
    auto a_data = make_shared<adafs_data>();
    //set the logger and initialize it with spdlog
    a_data->logger = spdlog::basic_logger_mt("basic_logger", "adafs.log");
    //extract the rootdir from argv and put it into rootdir of adafs_data
    a_data->rootdir = string(realpath(argv[argc-2], NULL));
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    //init fuse and give the private data for further reference.
    return fuse_main(argc, argv, &adafs_ops, a_data.get());
}
