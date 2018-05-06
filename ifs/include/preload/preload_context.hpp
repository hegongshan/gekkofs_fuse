#ifndef IFS_PRELOAD_CTX_HPP
#define IFS_PRELOAD_CTX_HPP

#include <extern/spdlog/spdlog.h>
#include <memory>
#include <string>


class PreloadContext {
    private:
    PreloadContext() = default;

    std::shared_ptr<spdlog::logger> log_;
    std::string mountdir_;

    public:
    static PreloadContext* getInstance() {
        static PreloadContext instance;
        return &instance;
    }

    PreloadContext(PreloadContext const&) = delete;
    void operator=(PreloadContext const&) = delete;

    void log(std::shared_ptr<spdlog::logger> logger);
    std::shared_ptr<spdlog::logger> log() const;
};


#endif //IFS_PRELOAD_CTX_HPP

