//
// Created by evie on 7/12/17.
//

#include "../../../main.hpp"
#include "../fuse_ops.hpp"
#include "../db/db_ops.hpp"
#include "../adafs_ops/mdata_ops.hpp"
#include "../rpc/client/c_data.hpp"
#include "../adafs_ops/io.hpp"

using namespace std;

/**
 * Read data
 *
 * Read should send exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the file
 * has been opened in 'direct_io' mode, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * fi->fh will contain the value set by the open method, or will
 * be undefined if the open method didn't set any value.
 *
 * Valid replies:
 *   fuse_reply_buf
 *   fuse_reply_iov
 *   fuse_reply_data
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param size number of bytes to read
 * @param off offset to read from
 * @param fi file information
 */
void adafs_ll_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info* fi) {
    // TODO Check out how splicing works. This uses fuse_reply_data
    ADAFS_DATA->spdlogger()->debug("adafs_ll_read() enter: inode {} size {} offset {}", ino, size, off);
    size_t read_size;
    auto buf = make_unique<char[]>(size);
    int err;
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(fmt::FormatInt(ino).str());
        if (ADAFS_DATA->is_local_op(recipient)) { // local read operation
            auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
            chnk_path /= fmt::FormatInt(ino).c_str();
            chnk_path /= "data"s;
            err = read_file(buf.get(), read_size, chnk_path.c_str(), size, off);
        } else { // remote read operation
            err = rpc_send_read(recipient, ino, size, off, buf.get(), read_size);
        }
    } else { //single node operation
        auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
        chnk_path /= fmt::FormatInt(ino).c_str();
        chnk_path /= "data"s;
        err = read_file(buf.get(), read_size, chnk_path.c_str(), size, off);
    }
    if (err != 0) {
        fuse_reply_err(req, EIO);
        return;
    }

    ADAFS_DATA->spdlogger()->trace("Sending buf to Fuse driver: {}", buf.get());
    fuse_reply_buf(req, buf.get(), read_size);
}


/**
 * Write data
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the file has
 * been opened in 'direct_io' mode, in which case the return value
 * of the write system call will reflect the return value of this
 * operation.
 *
 * Unless FUSE_CAP_HANDLE_KILLPRIV is disabled, this method is
 * expected to reset the setuid and setgid bits.
 *
 * fi->fh will contain the value set by the open method, or will
 * be undefined if the open method didn't set any value.
 *
 * Valid replies:
 *   fuse_reply_write
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param buf data to write
 * @param size number of bytes to write
 * @param off offset to write to
 * @param fi file information
 */
void
adafs_ll_write(fuse_req_t req, fuse_ino_t ino, const char* buf, size_t size, off_t off, struct fuse_file_info* fi) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_write() enter: inode {} size {} offset {} buf {} O_APPEND {}", ino, size,
                                   off,
                                   buf, fi->flags & O_APPEND);
    // TODO Check out how splicing works. This uses the function write_buf then.
    int err;
    size_t write_size;
    if (ADAFS_DATA->host_size() > 1) { // multiple node operation
        auto recipient = RPC_DATA->get_rpc_node(fmt::FormatInt(ino).str());
        if (ADAFS_DATA->is_local_op(recipient)) { // local write operation
            err = write_file(ino, buf, write_size, size, off, (fi->flags & O_APPEND) != 0);
        } else { // remote write operation
            err = rpc_send_write(recipient, ino, size, off, buf, write_size, (fi->flags & O_APPEND) != 0);
        }
    } else { //single node operation
        err = write_file(ino, buf, write_size, size, off, (fi->flags & O_APPEND) != 0);
    }
    if (err != 0) {
        fuse_reply_err(req, EIO);
        return;
    }

    fuse_reply_write(req, size);
}
