#include "logger.hpp"

namespace samos::log::logger
{

void set_level(LogLevel level)
{
    spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
}

} // namespace samos::log::logger
