//
// Created by evie on 9/4/17.
//

#include <daemon/fs_operations.hpp>
#include <adafs_ops/metadentry.hpp>
#include <adafs_ops/data.hpp>
#include <rpc/sender/c_metadentry.hpp>
#include <rpc/sender/c_data.hpp>

using namespace std;

int adafs_open(string& path, int flags, mode_t mode) {
    auto uid = getuid(); // XXX this should go into daemon init and set it globally
    auto gid = getgid();
    int ret;

    if (flags & O_CREAT) { // do file create TODO handle all other flags
        if (ADAFS_DATA->host_size() > 1) { // multiple node operation
            auto recipient = RPC_DATA->get_rpc_node(path);
            if (ADAFS_DATA->is_local_op(recipient)) { // local
                ret = create_node(path, uid, gid, mode);
            } else { // remote
                ret = rpc_send_create_node(recipient, path, mode);
            }
        } else { // single node operation
            ret = create_node(path, uid, gid, mode);
        }
    } else {
        // do nothing.
        ret = 0;
    }
    return ret;
}

FILE* adafs_fopen(string& path, const char* mode) {
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path);
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

int adafs_unlink(string& path) {
    int ret;
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            ret = remove_node(path);
        } else { // remote
            ret = rpc_send_remove_node(recipient, path);
        }
    } else { // single node operation
        ret = remove_node(path);
    }
    return ret;
}

int adafs_close(string& path) {
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path);
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

int adafs_stat(string& path, struct stat* buf) {
    int ret;
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            ret = get_attr(path, buf);
        } else { // remote
            ret = rpc_send_get_attr(recipient, path, buf);
        }
    } else { // single node operation
        ret = get_attr(path, buf);
    }
    return ret;
}

/**
 * write buf to path
 * @param path
 * @param buf
 * @param size
 * @param offset
 * @return
 */
ssize_t adafs_write(string& path, char* buf, size_t size, off_t offset) {
    // TODO make sure this function does everything it should do. recheck if buffer should be const or not
    size_t write_size;
    int err;
    // XXX Append is set when opening the file. This needs to get here somehow. this is why false is hardcoded for now
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            err = write_file(path, buf, write_size, size, offset, false);
        } else { // remote
            err = rpc_send_write(recipient, path, size, offset, buf, write_size, false);
        }
    } else { // single node operation
        err = write_file(path, buf, write_size, size, offset, false);
    }
    return err;
}

/**
 * If offset is nonzero than it can be interpreted as pread
 * @param path
 * @param buf
 * @param size
 * @param offset
 * @return
 */
ssize_t adafs_read(string& path, char* buf, size_t size, off_t offset) {
    // TODO make sure this function does everything it should do. recheck if buffer should be const or not
    size_t read_size;
//    auto buf = make_unique<char[]>(size);
    int err;
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            err = read_file(buf, read_size, path, size, offset);
        } else { // remote
            err = rpc_send_read(recipient, path, size, offset, buf, read_size);
        }
    } else { // single node operation
        err = read_file(buf, read_size, path, size, offset);
    }
    return err;
}
