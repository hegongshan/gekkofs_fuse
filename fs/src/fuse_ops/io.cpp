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
int adafs_read(const char* p, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->info("##### FUSE FUNC ###### adafs_read() enter: name '{}' buf {} size {} offset {}", p, buf,
                             size, offset);
    // TODO implement
    return 0;
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
int adafs_write(const char* p, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->info("##### FUSE FUNC ###### adafs_write() enter: name '{}' buf {} size {} offset {}", p, buf,
                             size, offset);
    // TODO implement
    return 0;
}

/** Change the size of a file
 *
 * `fi` will always be NULL if the file is not currenly open, but
 * may also be NULL if the file is open.
 *
 * Unless FUSE_CAP_HANDLE_KILLPRIV is disabled, this method is
 * expected to reset the setuid and setgid bits.
 */
int adafs_truncate(const char* p, off_t offset, struct fuse_file_info* fi) {
    ADAFS_DATA->logger->info("##### FUSE FUNC ###### adafs_truncate() enter: name '{}' offset (size) {}", p, offset);

    auto path = bfs::path(p);
    auto path_hash = ADAFS_DATA->hashf(path.string());
    auto md = make_shared<Metadata>();

    get_metadata(*md, path);
    // XXX might need to check access here. But shouldn't access be checked through adafs_open() before?

    // Check the file size limits
    if (offset > numeric_limits<uint32_t>::max()) return -EFBIG;

    // Set new size of the file
    md->size((uint32_t) offset);

    // write it to disk (XXX make it dirty later and write to disk in fsync (I believe))
    write_metadata_field(md->size(), path_hash, md_field_map.at(Md_fields::size));

#ifdef ACMtime
    md->update_ACM_time(true, false, true);
    write_metadata_field(md->atime(), path_hash, md_field_map.at(Md_fields::atime));
    write_metadata_field(md->mtime(), path_hash, md_field_map.at(Md_fields::mtime));
#endif

    return 0;
}