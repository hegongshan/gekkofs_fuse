#ifndef GEKKOFS_CLIENT_FORWARD_DATA_HPP
#define GEKKOFS_CLIENT_FORWARD_DATA_HPP

namespace gkfs::rpc {

struct ChunkStat {
    unsigned long chunk_size;
    unsigned long chunk_total;
    unsigned long chunk_free;
};

// TODO once we have LEAF, remove all the error code returns and throw them as
// an exception.

std::pair<int, ssize_t>
forward_write(const std::string& path, const void* buf, bool append_flag,
              off64_t in_offset, size_t write_size,
              int64_t updated_metadentry_size);

std::pair<int, ssize_t>
forward_read(const std::string& path, void* buf, off64_t offset,
             size_t read_size);

int
forward_truncate(const std::string& path, size_t current_size, size_t new_size);

std::pair<int, ChunkStat>
forward_get_chunk_stat();

} // namespace gkfs::rpc

#endif // GEKKOFS_CLIENT_FORWARD_DATA_HPP
