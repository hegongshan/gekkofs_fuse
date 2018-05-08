#ifndef IFS_PRELOAD_CTX_HPP
#define IFS_PRELOAD_CTX_HPP

#include <extern/spdlog/spdlog.h>
#include <memory>
#include <string>

/* Forward declarations */
class OpenFileMap;


class PreloadContext {
    private:
    PreloadContext();

    std::shared_ptr<spdlog::logger> log_;
    std::shared_ptr<OpenFileMap> ofm_;
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

    void mountdir(const std::string& path);
    std::string mountdir() const;

    bool relativize_path(std::string& path) const;

    const std::shared_ptr<OpenFileMap>& file_map() const;
};


#endif //IFS_PRELOAD_CTX_HPP

