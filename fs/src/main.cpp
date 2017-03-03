#include "main.h"
#include "fuse_ops.h"

#include <string.h>

static struct fuse_operations adafs_ops;



//TODO Implement all stat functions so that getattr works as expected
//bb_getattr(path="/", statbuf=0x8b9a8c60)
//bb_fullpath:  rootdir = "/home/draze/ownCloud/Promotion/gogs_git/ada-fs/bbfs/playground/mountdir", path = "/", fpath = "/home/draze/ownCloud/Promotion/gogs_git/ada-fs/bbfs/playground/mountdir/"
//lstat returned 0
//si:
//        st_dev = 45
//st_ino = 4328661
//st_mode = 040755
//st_nlink = 2
//st_uid = 1000
//st_gid = 1000
//st_rdev = 0
//st_size = 4096
//st_blksize = 4096
//st_blocks = 8
//st_atime = 0x58b1ffb8
//st_mtime = 0x58b6cfc9
//st_ctime = 0x58b6cfc9
int adafs_getattr(const char *path, struct stat *attr){

    if (strcmp(path, "/") == 0) {
        attr->st_mode = S_IFDIR | 0755;
        attr->st_nlink = 2;
    }

    if (strcmp(path, "/file") == 0) {
        attr->st_mode = S_IFREG | 0755;
        attr->st_nlink = 1;
        attr->st_size = strlen("blubb");
        ADAFS_DATA->logger->info(ADAFS_DATA->rootdir);
        return 0;
    }
    return -ENOENT;
}

void *adafs_init(struct fuse_conn_info *conn) {
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


#include <iostream>
#include <memory>
int main(int argc, char *argv[]) {
    //Initialize the mapping of Fuse functions
    init_adafs_ops(&adafs_ops);

    // create the adafs_data object (struct)
    auto a_data = std::make_shared<adafs_data>();
    //set the logger and initialize it with spdlog
    a_data->logger = spdlog::basic_logger_mt("basic_logger", "adafs.log");
    //extract the rootdir from argv and put it into rootdir of adafs_data
    a_data->rootdir = std::string(realpath(argv[argc-2], NULL));
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    //init fuse and give the private data for further reference.
    return fuse_main(argc, argv, &adafs_ops, a_data.get());
}
