#include "main.h"
#include "fuse_ops.h"

#include <string.h>
#include <sys/errno.h>

static struct fuse_operations adafs_ops;

int adafs_getattr(const char *path, struct stat *attr){

    if (strcmp(path, "/") == 0) {
        attr->st_mode = S_IFDIR | 0755;
        attr->st_nlink = 2;
    }

    if (strcmp(path, "/file") == 0) {
        attr->st_mode = S_IFREG | 0755;
        attr->st_nlink = 1;
        attr->st_size = strlen("blubb");
        return 0;
    }

    return -ENOENT;
}


static void init_adafs_ops(fuse_operations *ops) {
    ops->getattr = adafs_getattr;
}


#include <sys/param.h>
int main(int argc, char *argv[]) {

    init_adafs_ops(&adafs_ops);

    return fuse_main(argc, argv, &adafs_ops, nullptr);
}
