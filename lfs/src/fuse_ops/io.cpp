//
// Created by evie on 7/12/17.
//

#include "../main.hpp"
#include "../fuse_ops.hpp"
#include "../db/db_ops.hpp"
#include "../adafs_ops/mdata_ops.hpp"

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
    \
    ADAFS_DATA->spdlogger()->debug("adafs_ll_read() enter: inode {} size {} offset {}", ino, size, off);
    // TODO Check out how splicing works. This uses fuse_reply_data
    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= fmt::FormatInt(ino).c_str();
    chnk_path /= "data"s;

    int fd = open(chnk_path.c_str(), R_OK);

    if (fd < 0) {
        fuse_reply_err(req, EIO);
        return;
    }
    char* buf = new char[size]();

    auto read_size = static_cast<size_t>(pread(fd, buf, size, off));
    ADAFS_DATA->spdlogger()->trace("Read the following buf: {}", buf);

    fuse_reply_buf(req, buf, read_size);


    close(fd);
    free(buf);
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
    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path());
    chnk_path /= fmt::FormatInt(ino).c_str();
    chnk_path /= "data"s;

    int fd = open(chnk_path.c_str(), W_OK);

    if (fd < 0) {
        fuse_reply_err(req, EIO);
        return;
    }
    // write to disk
    pwrite(fd, buf, size, off);

    // Set new size of the file
    if (fi->flags & O_APPEND) {
        // appending requires to read the old size first so that the new size can be added to it
        Metadata md{};
        read_metadata_field_md(ino, Md_fields::size, md);
        // truncating file
        truncate(chnk_path.c_str(), md.size() + size);
        // refresh metadata size field
        write_metadata_field(ino, Md_fields::size, md.size() + static_cast<off_t>(size));
    } else {
        truncate(chnk_path.c_str(), size);
        write_metadata_field(ino, Md_fields::size, static_cast<off_t>(size));
    }

    fuse_reply_write(req, size);
    close(fd);
}
