#include "lib_samos.hpp"
#include "../samos_config.hpp"
#include "option_parser.hpp"
#include "logger.hpp"
#include "ed_line.hpp"
#include "kelyphos.hpp"

#include <string>
#include <type_traits>
#include <fmt/core.h>

namespace samos_ui = samos::user_interface;
namespace samos_args = samos_ui::option_parser;
namespace slog = samos::log::logger;
using SLogLevel = slog::LogLevel;

void version_callback()
{
    std::cout << PROJECT_NAME << " " << PROJECT_VER << "\n";
}

int main(int argc, char** argv)
{
    slog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    samos_args::OptionParser samos_opts(argc, argv, "samos", "High fidelity interactive orbital simulation tool");

    auto flags_added = samos_opts.add_flag_set({
        {"k", "kelyphos", "Run The Kelyphos Shell", run_kelyphos},
        {"v", "version", "Param version", version_callback},
        {"h", "help", "Print usage", samos_opts.create_help_callback(version_callback)}
    });

    if (flags_added.is_err())
    {
        auto err = flags_added.get_err().format();
        fmt::print(stderr, "Failed to add flags {}\n", err);
        slog::log(SLogLevel::Critical, "Failed to add flags: {}", err);
        std::exit(1);
    }

    auto parsed_res = samos_opts.parse();

    if (parsed_res.is_err())
    {
        auto err = parsed_res.get_err().format();
        fmt::print(stderr, "Failed to parse arguments: {}\n", err);
        slog::log(SLogLevel::Critical, "Failed to parse arguments: {}", err);
        std::exit(1);
    }

    if (samos_opts.handle_flag("help") || samos_opts.handle_flag("version"))
    {
        std::exit(0);
    }

    samos_opts.handle_flag("kelyphos");

    return 0;
}
