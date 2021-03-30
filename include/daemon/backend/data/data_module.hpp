#ifndef GEKKOFS_DAEMON_DATA_LOGGING_HPP
#define GEKKOFS_DAEMON_DATA_LOGGING_HPP

#include <spdlog/spdlog.h>

namespace gkfs {
namespace data {

class DataModule {

private:
    DataModule() {}

    std::shared_ptr<spdlog::logger> log_;

public:
    static constexpr const char* LOGGER_NAME = "DataModule";

    static DataModule*
    getInstance() {
        static DataModule instance;
        return &instance;
    }

    DataModule(DataModule const&) = delete;

    void
    operator=(DataModule const&) = delete;

    const std::shared_ptr<spdlog::logger>&
    log() const;

    void
    log(const std::shared_ptr<spdlog::logger>& log);
};

#define GKFS_DATA_MOD                                                          \
    (static_cast<gkfs::data::DataModule*>(                                     \
            gkfs::data::DataModule::getInstance()))

} // namespace data
} // namespace gkfs

#endif // GEKKOFS_DAEMON_DATA_LOGGING_HPP
