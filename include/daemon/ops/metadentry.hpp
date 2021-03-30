#ifndef GEKKOFS_METADENTRY_HPP
#define GEKKOFS_METADENTRY_HPP

#include <daemon/daemon.hpp>
#include <global/metadata.hpp>

namespace gkfs {
namespace metadata {

Metadata
get(const std::string& path);

std::string
get_str(const std::string& path);

size_t
get_size(const std::string& path);

std::vector<std::pair<std::string, bool>>
get_dirents(const std::string& dir);

std::vector<std::tuple<std::string, bool, size_t, time_t>>
get_dirents_extended(const std::string& dir);

void
create(const std::string& path, Metadata& md);

void
update(const std::string& path, Metadata& md);

void
update_size(const std::string& path, size_t io_size, off_t offset, bool append);

void
remove(const std::string& path);

} // namespace metadata
} // namespace gkfs

#endif // GEKKOFS_METADENTRY_HPP
