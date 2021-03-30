#include <daemon/backend/data/data_module.hpp>

namespace gkfs::data {

const std::shared_ptr<spdlog::logger>&
DataModule::log() const {
    return log_;
}

void
DataModule::log(const std::shared_ptr<spdlog::logger>& log) {
    DataModule::log_ = log;
}

} // namespace gkfs::data