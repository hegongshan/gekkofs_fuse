#include <preload/preload_context.hpp>

#include <extern/spdlog/spdlog.h>

void PreloadContext::log(std::shared_ptr<spdlog::logger> logger) {
    log_ = logger;
}

std::shared_ptr<spdlog::logger> PreloadContext::log() const {
    return log_;
}

void PreloadContext::mountdir(const std::string& path) {
    mountdir_ = path;
}

std::string PreloadContext::mountdir() const {
    return mountdir_;
}

