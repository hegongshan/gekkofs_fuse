#include <preload/preload_context.hpp>

#include <extern/spdlog/spdlog.h>

void PreloadContext::log(std::shared_ptr<spdlog::logger> logger) {
    log_ = logger;
}

std::shared_ptr<spdlog::logger> PreloadContext::log() const {
    return log_;
}
