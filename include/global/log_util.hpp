#ifndef GEKKOFS_LOG_UITIL_HPP
#define GEKKOFS_LOG_UITIL_HPP

#include <spdlog/spdlog.h>

namespace gkfs::log {

spdlog::level::level_enum
get_level(std::string level_str);

spdlog::level::level_enum
get_level(unsigned long level);

void
setup(const std::vector<std::string>& loggers, spdlog::level::level_enum level,
      const std::string& path);
} // namespace gkfs::log

#endif
