#ifndef SAMOS_LOGGER_HPP
#define SAMOS_LOGGER_HPP

#ifndef SPDLOG_FMT_EXTERNAL
#define SPDLOG_FMT_EXTERNAL
#endif

#include "spdlog/spdlog.h"
#include "spdlog/common.h"
#include "fmt/core.h"
#include <string>

namespace samos::log::logger
{

enum class LogLevel
{
    Trace = spdlog::level::trace,
    Debug = spdlog::level::debug,
    Info = spdlog::level::info,
    Warn = spdlog::level::warn,
    Error = spdlog::level::err,
    Critical = spdlog::level::critical,
    Off = spdlog::level::off
};


template<typename FormatString, typename ... Args>
void log(LogLevel level, const FormatString& fmt, Args&&... args)
{
    switch (level)
    {
    case LogLevel::Info:
        spdlog::info(fmt, std::forward<Args>(args)...);
        break;

    case LogLevel::Debug:
        spdlog::debug(fmt, std::forward<Args>(args)...);
        break;

    case LogLevel::Warn:
        spdlog::warn(fmt, std::forward<Args>(args)...);
        break;

    case LogLevel::Error:
        spdlog::error(fmt, std::forward<Args>(args)...);
        break;

    case LogLevel::Critical:
        spdlog::critical(fmt, std::forward<Args>(args)...);
        break;

    default:
        break;
    }
}

template<typename FormatString>
void set_pattern(const FormatString& fmt)
{
    spdlog::set_pattern(fmt);
}

void set_level(LogLevel level);

}

#endif // SAMOS_LOGGER_HPP
