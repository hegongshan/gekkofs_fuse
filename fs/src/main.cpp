#include "main.h"
#include "metadata.h"
#include "metadata_ops.h"
#include "fuse_ops.h"

static struct fuse_operations adafs_ops;

using namespace std;


int adafs_getattr(const char *path, struct stat *attr, struct fuse_file_info *fi){
    //FUSE_UNKNOWN_INO errorcode if attr->st_ino could not be resolved by VFS (This is unverified information
    // but Fuse initializes it with this value initially fuse.c:3303

//    if (attr->st_ino == -1) {
//        ADAFS_DATA->logger->info("st_ino = -1: "s + to_string(attr->st_ino));
//    } else {
//        ADAFS_DATA->logger->info("st_ino != -1: "s + to_string(attr->st_ino));
//    }

//    ADAFS_DATA->logger->info("adafs_getattr_enter1"s);
//    ADAFS_DATA->logger->info("st_ino: "s + to_string(attr->st_ino));
//    ADAFS_DATA->logger->info("adafs_getattr_enter2"s);
    // call lookup for *path, return int (use errorcodes), put pointer to Metadata object in parameter
    // if exist (i.e. == 0) use Metadata object, else return ENOENT
//    auto path_s = bfs::path(path);
//    ADAFS_DATA->logger->info(path_s);
//    ADAFS_DATA->logger->flush();
    auto md = make_shared<Metadata>();
//    md->mode(S_IFDIR | 0755);
//    md->inode_no(1);
//    get_metadata(*md, path_s);
//    read_all_metadata(*md, 1, fpath);
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
        attr->st_mode = S_IFDIR | 0755;
        return 0;
    }
    if (strcmp(path, "/file/file2") == 0) {
        auto p_dir = make_shared<struct stat>();
        lstat("/", p_dir.get());
        ADAFS_DATA->logger->info(p_dir->st_ino);
        ADAFS_DATA->logger->flush();

        attr->st_mode = S_IFREG | 0755;
        attr->st_nlink = 1;
        attr->st_size = strlen("blubb");
        return 0;
    }
    return -ENOENT;
}

void *adafs_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {

//    ADAFS_DATA->logger->info("init function"s);
//    ADAFS_DATA->logger->info("uid_: {}", fuse_get_context()->uid);
//    ADAFS_DATA->logger->info("gid_: {}", fuse_get_context()->gid);
//    ADAFS_DATA->logger->info("pid: {0:d}", fuse_get_context()->pid);
//    ADAFS_DATA->logger->info("rootdir: {}", ((struct adafs_data*)fuse_get_context()->private_data)->rootdir);
    //Initialize directory structure for metadata. XXX We need this later for lowlevel when we split dentry from inodes
    boost::filesystem::create_directories(ADAFS_DATA->rootdir + "/meta/dentries"s);
    boost::filesystem::create_directories(ADAFS_DATA->rootdir + "/meta/inodes"s);
    boost::filesystem::create_directories(ADAFS_DATA->rootdir + "/data/chunks"s);

    // XXX Dunno if this is of any use
    cfg->use_ino = 1;

    //Init file system configuration
    ADAFS_DATA->blocksize = 4096;

    // Init unordered_map for caching metadata that was already looked up XXX use later
    ADAFS_DATA->hashmap = unordered_map<string, string>();
    ADAFS_DATA->hashf = hash<string>();

    auto md = make_shared<Metadata>();

    // Check that root metadata exists. If not intiialize it
    if (get_metadata(*md, "/"s) == -ENOENT) {
        ADAFS_DATA->logger->info("Root metadata not found. Initializing..."s);
        md->init_ACM_time();
        md->mode(S_IFDIR | 0755);
        md->uid(fuse_get_context()->uid);
        md->gid(fuse_get_context()->gid);
        md->inode_no(ADAFS_ROOT_INODE);
        write_all_metadata(*md, ADAFS_DATA->hashf("/"s), ADAFS_DATA->inode_path);
        ADAFS_DATA->logger->info("Creating Metadata object"s);
    }

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

    // create the private data struct
    auto a_data = make_shared<adafs_data>();
    //set the logger and initialize it with spdlog
    a_data->logger = spdlog::basic_logger_mt("basic_logger", "adafs.log");
    //extract the rootdir from argv and put it into rootdir of adafs_data
    a_data->rootdir = string(realpath(argv[argc-2], NULL));
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    //set all paths
    a_data->inode_path = a_data->rootdir + "meta/inodes"s;
    a_data->dentry_path = a_data->rootdir + "meta/dentries"s;
    a_data->chunk_path = a_data->rootdir + "data/chunks"s;
    //print version
    cout << "Fuse library version: "s + to_string(FUSE_MAJOR_VERSION) + to_string(FUSE_MINOR_VERSION) << endl;
    //init fuse and give the private data struct for further reference.
    return fuse_main(argc, argv, &adafs_ops, a_data.get());
}
