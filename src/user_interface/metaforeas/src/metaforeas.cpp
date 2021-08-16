#include "metaforeas.hpp"
#include "logger.hpp"

#include <fmt/core.h>
#include <string>

namespace samos::user_interface::metaforeas
{

Metaforeas::Metaforeas(int argc, char** argv)
    :
    option_parser{argc, argv, "Metaforeas", "Something"},
    schemer{},
    filenames{}
{
}

void Metaforeas::run()
{
    auto res = option_parser.add_container_flag<std::string>({"f", "file", "file to be run", {}});

    if (res.is_err())
    {
        log::logger::log(log::logger::LogLevel::Error, "Error: {}", res.get_err().format());
        return;
    }

    res = option_parser.parse();
    if (res.is_err())
    {
        log::logger::log(log::logger::LogLevel::Error, "Error: {}", res.get_err().format());
        return;
    }

    auto filenames_opt = option_parser.flag_value<std::vector<std::string>>("file");

    if (filenames_opt.has_value())
    {
        filenames = filenames_opt.value();
    }

    log::logger::log(log::logger::LogLevel::Debug, "files: {}", filenames.size());

    for (auto filename : filenames)
    {
        log::logger::log(log::logger::LogLevel::Info, "Info string {}", filename);
        schemer.load(filename);
    }
}

} // namespace samos::user_interface::metaforeas
