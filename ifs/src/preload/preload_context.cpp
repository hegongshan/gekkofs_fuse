#include <preload/preload_context.hpp>

#include <preload/open_file_map.hpp>
#include <extern/spdlog/spdlog.h>
#include <global/path_util.hpp>
#include <cassert>


PreloadContext::PreloadContext():
    ofm_(std::make_shared<OpenFileMap>()),
    fs_conf_(std::make_shared<FsConfig>())
{}

void PreloadContext::log(std::shared_ptr<spdlog::logger> logger) {
    log_ = logger;
}

std::shared_ptr<spdlog::logger> PreloadContext::log() const {
    return log_;
}

void PreloadContext::mountdir(const std::string& path) {
    assert(is_absolute_path(path));
    mountdir_ = path;
}

std::string PreloadContext::mountdir() const {
    return mountdir_;
}

bool PreloadContext::relativize_path(std::string& path) const {
    if(mountdir_.empty()) {
        /* Relativize path has been called before the library constructor has been invoked
         * thus the mountdir has not been set yet
         */
        return false;
    }

    if(!is_absolute_path(path)) {
        /* We don't support path resolution at the moment
         * thus we don't know how to handle relative path
         */
        return false;
    }

    path = path_to_relative(mountdir_, path);
    return !path.empty();
}

const std::shared_ptr<OpenFileMap>& PreloadContext::file_map() const {
    return ofm_;
}

const std::shared_ptr<FsConfig>& PreloadContext::fs_conf() const {
    return fs_conf_;
}

