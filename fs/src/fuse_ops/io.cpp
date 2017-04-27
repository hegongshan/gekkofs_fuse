//
// Created by evie on 3/30/17.
//

#include "../main.h"
#include "../fuse_ops.h"
#include "../adafs_ops/metadata_ops.h"

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.	 An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 */
// XXX error handling
int adafs_read(const char* p, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_read() enter: name '{}' size {} offset {}", p,
                              size, offset);
    auto path = bfs::path(p);
    auto md = make_shared<Metadata>();
    // access is already checked before through adafs_open()
    // get metadata and make sure file exists
    get_metadata(*md, path);

    // XXX the chunk path, i.e., the single file that is used for storing the data for now
    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path);
    chnk_path /= to_string(ADAFS_DATA->hashf(path.string()));
    chnk_path /= "data"s;

    int fd = open(chnk_path.c_str(), R_OK);

    if (fd < 0) return -1;

    auto new_size = static_cast<int>(pread(fd, buf, size, offset));

    close(fd);

    return new_size;
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.	 An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Unless FUSE_CAP_HANDLE_KILLPRIV is disabled, this method is
 * expected to reset the setuid and setgid bits.
 */
// XXX error handling
int adafs_write(const char* p, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->debug("##### FUSE FUNC ###### adafs_write() enter: name '{}' O_APPEND {} size {} offset {}",
                              p, fi->flags & O_APPEND, size, offset);
    auto path = bfs::path(p);
    auto path_hash = ADAFS_DATA->hashf(path.string());

    auto md = make_shared<Metadata>();
    // access is already checked before through adafs_open()
    // get metadata and make sure file exists
    get_metadata(*md, path);

    // XXX the chunk path, i.e., the single file that is used for storing the data for now
    auto chnk_path = bfs::path(ADAFS_DATA->chunk_path);
    chnk_path /= to_string(ADAFS_DATA->hashf(path.string()));
    chnk_path /= "data"s;

    int fd = open(chnk_path.c_str(), W_OK);

    if (fd < 0) return -1;

    pwrite(fd, buf, size, offset);

    // Set new size of the file
    if (fi->flags & O_APPEND) {
        truncate(chnk_path.c_str(), md->size() + size);
        md->size(md->size() + static_cast<uint32_t>(size));
    } else {
        truncate(chnk_path.c_str(), size);
        md->size(static_cast<uint32_t>(size));
    }

    close(fd);

    // write it to disk (XXX make it dirty later and write to disk in fsync (I believe))
    write_metadata_field(md->size(), path_hash, md_field_map.at(Md_fields::size));

#ifdef ACMtime
    md->update_ACM_time(true, true, true);
    write_metadata_field(md->atime(), path_hash, md_field_map.at(Md_fields::atime));
    write_metadata_field(md->ctime(), path_hash, md_field_map.at(Md_fields::ctime));
    write_metadata_field(md->mtime(), path_hash, md_field_map.at(Md_fields::mtime));
#endif

    auto new_size = static_cast<int>(size);

    return new_size;
}

/** Change the size of a file
 *
 * `fi` will always be NULL if the file is not currenly open, but
 * may also be NULL if the file is open.
 *
 * Unless FUSE_CAP_HANDLE_KILLPRIV is disabled, this method is
 * expected to reset the setuid and setgid bits.
 */
// XXX I thought I know when it is called. But I have no idea really. Watch out in logs
int adafs_truncate(const char* p, off_t offset, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->info("##### FUSE FUNC ###### adafs_truncate() enter: name '{}' offset (size) {}", p, offset);
// XXX commented out. Comment in and modify when I know when truncate is actually called
//    auto path = bfs::path(p);
//    auto path_hash = ADAFS_DATA->hashf(path.string());
//    auto md = make_shared<Metadata>();
//
//    get_metadata(*md, path);
//    // XXX might need to check access here. But shouldn't access be checked through adafs_open() before?
//
//    // Check the file size limits
//    if (offset > numeric_limits<uint32_t>::max()) return -EFBIG;
//
//    // Set new size of the file
//    md->size((uint32_t) offset);
//
//    // write it to disk (XXX make it dirty later and write to disk in fsync (I believe))
//    write_metadata_field(md->size(), path_hash, md_field_map.at(Md_fields::size));
//
//#ifdef ACMtime
//    md->update_ACM_time(true, false, true);
//    write_metadata_field(md->atime(), path_hash, md_field_map.at(Md_fields::atime));
//    write_metadata_field(md->mtime(), path_hash, md_field_map.at(Md_fields::mtime));
//#endif

    return 0;
}
