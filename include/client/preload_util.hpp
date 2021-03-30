#ifndef GEKKOFS_PRELOAD_UTIL_HPP
#define GEKKOFS_PRELOAD_UTIL_HPP

#include <client/preload.hpp>
#include <global/metadata.hpp>

#include <string>
#include <iostream>
#include <map>
#include <type_traits>

namespace gkfs::metadata {

struct MetadentryUpdateFlags {
    bool atime = false;
    bool mtime = false;
    bool ctime = false;
    bool uid = false;
    bool gid = false;
    bool mode = false;
    bool link_count = false;
    bool size = false;
    bool blocks = false;
    bool path = false;
};

} // namespace gkfs::metadata

// Hermes instance
namespace hermes {
class async_engine;
}

extern std::unique_ptr<hermes::async_engine> ld_network_service;

// function definitions
namespace gkfs::utils {
template <typename E>
constexpr typename std::underlying_type<E>::type
to_underlying(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

std::optional<gkfs::metadata::Metadata>
get_metadata(const std::string& path, bool follow_links = false);

int
metadata_to_stat(const std::string& path, const gkfs::metadata::Metadata& md,
                 struct stat& attr);

void
load_hosts();

void
load_forwarding_map();

std::vector<std::pair<std::string, std::string>>
read_hosts_file();

void
connect_to_hosts(const std::vector<std::pair<std::string, std::string>>& hosts);

} // namespace gkfs::utils

#endif // GEKKOFS_PRELOAD_UTIL_HPP
