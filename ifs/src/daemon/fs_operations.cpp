//
// Created by evie on 9/4/17.
//

#include "daemon/fs_operations.hpp"

using namespace std;

int adafs_open(char* path, int flags, mode_t mode) {
    if (flags & O_CREAT) { // do file create TODO handle all other flags
        string path_s(path);
        if (ADAFS_DATA->host_size() > 1) { // multiple node operation
            auto recipient = RPC_DATA->get_rpc_node(path_s);
            if (ADAFS_DATA->is_local_op(recipient)) { // local
                // TODO this node does operation
            } else { // remote
                // TODO call rpc
            }
        } else { // single node operation
            // TODO
        }
    } else {
        // do nothing.
    }
    return 0;
}

FILE* adafs_fopen(char* path, const char* mode) {
    string path_s(path);
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path_s);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            // TODO this node does operation
        } else { // remote
            // TODO call rpc
        }
    } else { // single node operation
        // TODO
    }
    return nullptr;
}

int adafs_close(char* path) {
    string path_s(path);
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path_s);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            // TODO this node does operation
        } else { // remote
            // TODO call rpc
        }
    } else { // single node operation
        // TODO
    }
    return 0;
}

int adafs_stat(char* path, struct stat* buf) {
    string path_s(path);
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path_s);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            // TODO this node does operation
        } else { // remote
            // TODO call rpc
        }
    } else { // single node operation
        // TODO
    }
    return 0;
}

int adafs_fstat(char* path, struct stat* buf) {
    string path_s(path);
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path_s);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            // TODO this node does operation
        } else { // remote
            // TODO call rpc
        }
    } else { // single node operation
        // TODO
    }
    return 0;
}

ssize_t adafs_write(char* path, void* buf, size_t count) {
    string path_s(path);
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path_s);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            // TODO this node does operation
        } else { // remote
            // TODO call rpc
        }
    } else { // single node operation
        // TODO
    }
    return 0;
}

ssize_t adafs_read(char* path, void* buf, size_t count) {
    string path_s(path);
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path_s);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            // TODO this node does operation
        } else { // remote
            // TODO call rpc
        }
    } else { // single node operation
        // TODO
    }
    return 0;
}

ssize_t adafs_pread(char* path, void* buf, size_t count, off_t offset) {
    string path_s(path);
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path_s);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            // TODO this node does operation
        } else { // remote
            // TODO call rpc
        }
    } else { // single node operation
        // TODO
    }
    return 0;
}