//
// Created by evie on 9/4/17.
//

#include <daemon/fs_operations.hpp>
#include <adafs_ops/metadentry.hpp>
#include <adafs_ops/data.hpp>
#include <rpc/sender/c_metadentry.hpp>
#include <rpc/sender/c_data.hpp>

using namespace std;

int adafs_open(char* path, int flags, mode_t mode) {
    auto uid = getuid(); // XXX this should go into daemon init and set it globally
    auto gid = getgid();
    int ret;

    if (flags & O_CREAT) { // do file create TODO handle all other flags
        string path_s(path);
        if (ADAFS_DATA->host_size() > 1) { // multiple node operation
            auto recipient = RPC_DATA->get_rpc_node(path_s);
            if (ADAFS_DATA->is_local_op(recipient)) { // local
                ret = create_node(path_s, uid, gid, mode);
            } else { // remote
                ret = rpc_send_create_node(recipient, mode);
            }
        } else { // single node operation
            ret = create_node(path_s, uid, gid, mode);
        }
    } else {
        // do nothing.
        ret = 0;
    }
    return ret;
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
    int ret;
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path_s);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            ret = get_attr(path_s, buf);
        } else { // remote
            ret = rpc_send_get_attr(recipient, path_s, buf);
        }
    } else { // single node operation
        ret = get_attr(path_s, buf);
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
ssize_t adafs_write(char* path, char* buf, size_t size, off_t offset) {
    // TODO make sure this function does everything it should do. recheck if buffer should be const or not
    string path_s(path);
    size_t write_size;
    int err;
    // XXX Append is set when opening the file. This needs to get here somehow. this is why false is hardcoded for now
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path_s);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            err = write_file(path_s, buf, write_size, size, offset, false);
        } else { // remote
            err = rpc_send_write(recipient, path_s, size, offset, buf, write_size, false);
        }
    } else { // single node operation
        err = write_file(path_s, buf, write_size, size, offset, false);
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
ssize_t adafs_read(char* path, char* buf, size_t size, off_t offset) {
    // TODO make sure this function does everything it should do. recheck if buffer should be const or not
    size_t read_size;
//    auto buf = make_unique<char[]>(size);
    int err;

    string path_s(path);
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(path_s);
        if (ADAFS_DATA->is_local_op(recipient)) { // local
            err = read_file(buf, read_size, path_s, size, offset);
        } else { // remote
            err = rpc_send_read(recipient, path_s, size, offset, buf, read_size);
        }
    } else { // single node operation
        err = read_file(buf, read_size, path_s, size, offset);
    }
    return err;
}
