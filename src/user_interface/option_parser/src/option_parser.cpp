#include "option_parser.hpp"

#include "logger.hpp"

#include <iostream>
#include <fmt/core.h>
#include <utf8.h>

namespace samos::user_interface::option_parser {

namespace slog = log::logger;

using LogLevel = slog::LogLevel;

InvalidUtf8Error::InvalidUtf8Error(const std::string& input)
    : input{input}
{
}

std::string InvalidUtf8Error::format() const
{
    return fmt::format("Invalid UTF8 Sequence {}", input);
}

FlagSizeError::FlagSizeError(size_t size)
    : size(size)
{
}

std::string FlagSizeError::format() const
{
    return fmt::format("Bad flag size: {}", size);
}

OptSizeError::OptSizeError(size_t size)
    : size(size)
{
}

std::string OptSizeError::format() const
{
    return fmt::format("Bad opt size: {}", size);
}

DocSizeError::DocSizeError(size_t size)
    : size(size)
{
}

std::string DocSizeError::format() const
{
    return fmt::format("Bad doc size: {}", size);
}

std::string ExceptionError::format() const
{
    return *this;
}

std::string OptParserError::format() const
{
    if (std::holds_alternative<InvalidUtf8Error>(*this))
    {
        return std::get<InvalidUtf8Error>(*this).format();
    }
    else if (std::holds_alternative<FlagSizeError>(*this))
    {
        return std::get<FlagSizeError>(*this).format();
    }
    else if (std::holds_alternative<OptSizeError>(*this))
    {
        return std::get<OptSizeError>(*this).format();
    }
    else if (std::holds_alternative<DocSizeError>(*this))
    {
        return std::get<DocSizeError>(*this).format();
    }
    else
    {
        assert(std::holds_alternative<ExceptionError>(*this));
        return std::get<ExceptionError>(*this).format();
    }
}

OptionParser::OptionParser(int argc, char** argv, const std::string& name, const std::string& summary)
    :
    options_impl(name, summary),
    parsed_options(),
    stored_args(argc),
    stored_argv(argc),
    callback_map()
{
    for (int idx = 0; idx < argc; ++idx)
    {
        stored_args[idx] = argv[idx];
        stored_argv[idx] = const_cast<char*>(stored_args[idx].c_str());
    }
}

OptParserResult OptionParser::add_flag(const std::string& flag, const std::string& opt, const std::string& doc)
{
    const auto validator = [](const std::string& input)
    {
        if (!utf8::is_valid(input.begin(), input.end()))
        {
            return OptParserResult::err(InvalidUtf8Error{input});
        }
        return OptParserResult::ok({});
    };

    constexpr size_t MAX_UTF8_CHARACTER_SIZE = 4;

    if (flag.size() != 1)
    {
        return OptParserResult::err(FlagSizeError{flag.size()});
    }

    if (opt.size() <= 1)
    {
        return OptParserResult::err(OptSizeError{opt.size()});
    }

    if (doc.size() <= 1)
    {
        return OptParserResult::err(DocSizeError{doc.size()});
    }

    auto res = validator(flag);

    if (res.is_err())
    {
        return res;
    }

    res = validator(opt);
    if (res.is_err())
    {
        return res;
    }

    return validator(doc);
}

OptParserResult OptionParser::add_flag_set(std::vector<FlagSet>&& flag_set)
{
    callback_map.reserve(callback_map.size() + flag_set.size());

    try
    {
        auto option_adder = options_impl.add_options();
        for (auto& flag : flag_set)
        {
            auto res = add_flag(flag.flag, flag.opt, flag.doc);
            if (res.is_err())
            {
                return res;
            }
            option_adder(flag.flag + "," + flag.opt, flag.doc);
            callback_map.emplace(std::make_pair(flag.flag, flag.callback));
            callback_map.emplace(std::make_pair(flag.opt, flag.callback));
        }
    }
    catch (const cxxopts::OptionException& e)
    {
        slog::log(LogLevel::Critical, "Error adding arguments: {}", e.what());
        return OptParserResult::err(ExceptionError{e.what()});
    }

    return OptParserResult::ok({});
}

OptParserResult OptionParser::parse()
{
    try
    {
        int argc = stored_argv.size();
        char** argv = stored_argv.data();

        parsed_options.emplace(options_impl.parse(argc, argv));
    }
    catch (const cxxopts::OptionException& e)
    {
        slog::log(LogLevel::Error, "Error parsing: {}", e.what());
        std::cout << options_impl.help() << std::endl;
        return OptParserResult::err(ExceptionError{e.what()});
    }

    return OptParserResult::ok({});
}

std::string OptionParser::help()
{
    return options_impl.help();
}

OptCallback OptionParser::create_help_callback(OptCallback pre_actions) const
{
    return [this, pre_actions]()
    {
        pre_actions();
        std::cout << options_impl.help() << std::endl;
    };
}

bool OptionParser::handle_flag(std::string&& flag)
{
    if (parsed_options.has_value() && parsed_options.value().count(flag) && callback_map.contains(flag))
    {
        callback_map[flag]();
        return true;
    }

    return false;
}

}
