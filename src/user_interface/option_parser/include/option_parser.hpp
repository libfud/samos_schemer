#ifndef SAMOS_OPTION_PARSER_HPP
#define SAMOS_OPTION_PARSER_HPP

#include "result.hpp"
#include "logger.hpp"

#include <cxxopts.hpp>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace samos::user_interface::option_parser {

class InvalidUtf8Error {
public:
    InvalidUtf8Error(const std::string& input);
    std::string format() const;

private:
    std::string input;
};

class FlagSizeError {
public:
    FlagSizeError(size_t size);
    std::string format() const;

private:
    size_t size;
};

class OptSizeError {
public:
    OptSizeError(size_t size);
    std::string format() const;

private:
    size_t size;
};

class DocSizeError {
public:
    DocSizeError(size_t size);
    std::string format() const;

private:
    size_t size;
};

class ExceptionError : std::string {
    using std::string::string;
public:
    std::string format() const;
};

class OptParserError : public std::variant<InvalidUtf8Error, FlagSizeError, OptSizeError, DocSizeError, ExceptionError>
{
    using variant::variant;
public:
    std::string format() const;
};

using OptCallback = std::function<void (void)>;
using OptParserResult = result::Result<std::monostate, OptParserError>;

struct FlagSet
{
    std::string flag;
    std::string opt;
    std::string doc;
    OptCallback callback;
};

class OptionParser {
public:

    OptionParser() = delete;

    ~OptionParser() = default;

    OptionParser(int argc, char** argv, const std::string& name, const std::string& summary);

    OptParserResult add_flag(const std::string& flag, const std::string& opt, const std::string& doc);

    OptParserResult add_flag_set(std::vector<FlagSet>&& flag_set);

    template <typename T>
    OptParserResult add_container_flag(FlagSet&& flag_set)
    {
        try
        {
            auto option_adder = options_impl.add_options();
            auto res = add_flag(flag_set.flag, flag_set.opt, flag_set.doc);
            if (res.is_err())
            {
                return res;
            }
            option_adder(
                fmt::format("{},{}", flag_set.flag, flag_set.opt),
                flag_set.doc,
                cxxopts::value<std::vector<T>>());
            callback_map.emplace(std::make_pair(flag_set.flag, flag_set.callback));
            callback_map.emplace(std::make_pair(flag_set.opt, flag_set.callback));
        }
        catch (const cxxopts::OptionException& e)
        {
            log::logger::log(log::logger::LogLevel::Critical, "Error adding arguments: {}", e.what());
            return OptParserResult::err(ExceptionError{e.what()});
        }

        return OptParserResult::ok({});
    }

    OptParserResult parse();

    std::string help();

    OptCallback create_help_callback(OptCallback pre_actions = {}) const;

    bool handle_flag(std::string&& flag);

    template <typename T>
    std::optional<T> flag_value(std::string&& flag) const
    {
        if (parsed_options.has_value() && parsed_options.value().count(flag))
        {
            return parsed_options.value()[flag].as<T>();
        }

        return {};
    }

private:

    cxxopts::Options options_impl;

    std::optional<cxxopts::ParseResult> parsed_options;

    std::vector<std::string> stored_args;

    std::vector<char*> stored_argv;

    std::unordered_map<std::string, OptCallback> callback_map;
};

}

#endif // SAMOS_OPTION_PARSER_HPP
