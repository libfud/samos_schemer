#include "config_manager.hpp"
#include "logger.hpp"

namespace samos::config_manager
{

using log::logger::log;
using log::logger::LogLevel;

ConfigMap::ConfigMap()
    :
    config_map{},
    config_keys{}
{
}

ConfigMap::~ConfigMap()
{
}

ConfigMap::ConfigMap(const ConfigMap& rhs)
    :
    config_map{},
    config_keys{}
{
    for (auto key_ref : rhs.config_keys)
    {
        std::string key{key_ref};
        auto value_res = rhs.copy_property(key);
        assert(value_res.is_ok());
        auto value{std::move(value_res.get_ok())};
        if (std::holds_alternative<ConfigMap*>(value))
        {
            auto res = register_property(std::move(key), std::move(value));
            delete std::get<ConfigMap*>(value);
            assert(res.is_ok());
        }
        else
        {
            auto res = register_property(std::move(key), std::move(value));
            assert(res.is_ok());
        }
    }
}

ConfigMap::ConfigMap(ConfigMap&& rhs)
    :
    config_map(std::move(rhs.config_map)),
    config_keys(std::move(rhs.config_keys))
{
}

ConfigMap& ConfigMap::operator=(const ConfigMap& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    config_map.clear();
    config_keys.clear();
    for (auto key_ref : rhs.config_keys)
    {
        std::string key{key_ref};
        auto value_res = rhs.copy_property(key);
        auto value{std::move(value_res.get_ok())};
        if (std::holds_alternative<ConfigMap*>(value))
        {
            auto res = register_property(std::move(key), std::move(value));
            delete std::get<ConfigMap*>(value);
            assert(res.is_ok());
        }
        else
        {
            auto res = register_property(std::move(key), std::move(value));
            assert(res.is_ok());
        }
    }
    return *this;
}

ConfigMap& ConfigMap::operator=(ConfigMap&& rhs)
{
    config_map = std::move(rhs.config_map);
    config_keys = std::move(rhs.config_keys);
    return *this;
}

ConfigResult<> ConfigMap::register_property(std::string&& key, ConfigValue&& value)
{
    if (config_map.contains(key))
    {
        return ConfigResult<>::err(PropertyAlreadyRegistered{});
    }

    if (std::holds_alternative<ConfigMap*>(value))
    {
        config_map.emplace(std::make_pair(key, new ConfigMap{*std::get<ConfigMap*>(value)}));
    }
    else
    {
        config_map.emplace(std::make_pair(key, std::move(ConfigValue{value})));
    }

    config_keys.emplace(std::move(key));

    return ConfigResult<>::ok({});
}

ConfigResult<> ConfigMap::register_properties(
    std::vector<std::pair<std::string, ConfigValue>>&& key_value_pairs)
{
    for (auto k_v : key_value_pairs)
    {
        std::string key{k_v.first};
        auto res = register_property(std::move(key), std::move(k_v.second));
        if (res.is_err())
        {
            return res;
        }
    }

    return ConfigResult<>::ok({});
}

ConfigResult<> ConfigMap::set_property(const std::string& key, ConfigValue&& value)
{
    if (!config_map.contains(key))
    {
        return ConfigResult<>::err(PropertyNotRegistered{});
    }

    if (config_map.at(key).index() != value.index())
    {
        return ConfigResult<>::err(TypeError{});
    }

    if (std::holds_alternative<ConfigMap*>(value))
    {
        delete std::get<ConfigMap*>(config_map.at(key));
        // ConfigMap* new_value =
        config_map.at(key) = new ConfigMap{*std::get<ConfigMap*>(value)};
    }
    else
    {
        config_map.at(key) = value;
    }

    return ConfigResult<>::ok({});
}

ConfigResult<ConfigValue> ConfigMap::pop_property(std::string&& key)
{
    if (!config_map.contains(key))
    {
        return ConfigResult<ConfigValue>::err(PropertyNotRegistered{});
    }

    ConfigValue value = config_map.at(key);

    config_map.erase(key);
    config_keys.erase(std::move(key));
    return ConfigResult<ConfigValue>::ok(value);
}

ConfigResult<bool> ConfigMap::property_is_nested_map(const std::string& key) const
{
    if (!config_map.contains(key))
    {
        return ConfigResult<bool>::err(PropertyNotRegistered{});
    }

    return ConfigResult<bool>::ok(std::holds_alternative<ConfigMap*>(config_map.at(key)));
}

ConfigResult<ConfigValue> ConfigMap::copy_property(const std::string& key) const
{
    if (!config_map.contains(key))
    {
        return ConfigResult<ConfigValue>::err(PropertyNotRegistered{});
    }

    ConfigValue value;

    if (std::holds_alternative<ConfigMap*>(config_map.at(key)))
    {
        value = new ConfigMap{*std::get<ConfigMap*>(config_map.at(key))};
    }
    else
    {
        value = config_map.at(key);
    }

    return ConfigResult<ConfigValue>::ok(value);
}

ConfigResult<const ConfigValue*> ConfigMap::ref_property(const std::string& key) const
{
    if (!config_map.contains(key))
    {
        return ConfigResult<const ConfigValue*>::err(PropertyNotRegistered{});
    }

    return ConfigResult<const ConfigValue*>::ok(&config_map.at(key));
}

ConfigResult<> ConfigMap::load_from_sexp(sexp& sexp_config, scheme::Schemer& schemer)
{
    for (auto key : config_keys)
    {
        if (property_is_nested_map(key).get_ok())
        {
            // ConfigMap* nested_map =
        }
        else
        {
            auto new_value_res = schemer.assq(key, sexp_config);
            if (new_value_res.is_err())
            {
                log(LogLevel::Error,
                    "{} Error getting value for key {}: {}",
                    __LINE__,
                    key,
                    new_value_res.get_err().format());
                continue;
            }

            auto sexp_new_value = new_value_res.get_ok();
            auto cpp_value_res = schemer.get_cpp_value(sexp_new_value);
            if (cpp_value_res.is_err())
            {
                log(LogLevel::Error,
                    "{}: Expected C++ type value for key {}: {}\nvalue={}",
                    __LINE__,
                    key,
                    cpp_value_res.get_err().format(),
                    schemer.sexp_to_string(sexp_new_value)
                );
                continue;
            }

            auto cpp_value = cpp_value_res.get_ok();
            auto update_res = set_property(key, cpp_value);
            if (update_res.is_err())
            {
                log(LogLevel::Error,
                    "{} Error updating property for key {}: {}",
                    __LINE__,
                    key,
                    update_res.get_err().format());
                continue;
            }
            else
            {
                log(LogLevel::Info, "{} Updated property for key {}", __LINE__, key);
            }
        }
    }

    return ConfigResult<>::ok({});
}

const std::set<std::string>& ConfigMap::keys() const
{
    return config_keys;
}

ConfigNest::ConfigNest()
    :
    name{},
    default_map{},
    loaded_map{}
{
}

const std::string& ConfigNest::get_name() const
{
    return name;
}

ConfigResult<ConfigMap> ConfigNest::load_config_from_sexp(sexp& config_alist, scheme::Schemer& schemer)
{
    using ConfigMapResult = ConfigResult<ConfigMap>;
    auto sexp_res = schemer.assq(name, config_alist);

    if (sexp_res.is_err())
    {
        auto err = sexp_res.get_err();
        log(LogLevel::Error, "{} Error loading config from sexp {}", __LINE__, err.format());
        schemer.print_exception(config_alist);
        return ConfigMapResult::err(err);
    }

    sexp sexp_config = sexp_res.get_ok();
    auto config_type = schemer.sexp_type(sexp_config);

    if (config_type != scheme::SexpType::Pair)
    {
        return ConfigMapResult::err(NotConfigMap{sexp_config});
    }

    auto config_result = loaded_map.load_from_sexp(sexp_config, schemer);

    if (config_result.is_err())
    {
        return ConfigMapResult::err(config_result.get_err());
    }

    return ConfigMapResult::ok(loaded_map);
}

ConfigManager::ConfigManager()
    :
    root_map{},
    schemer{}
{
    schemer.import_module(scheme::SrfiType::Srfi_ListLibrary);
}

ConfigResult<ConfigMap> ConfigManager::load_config_from_file(const std::string& filename)
{
    using ConfigMapResult = ConfigResult<ConfigMap>;
    auto res = schemer.read_from_file(filename);

    if (res.is_err())
    {
        return ConfigMapResult::err(res.get_err());
    }

    sexp config_alist = res.get_ok();

    return root_map.load_config_from_sexp(config_alist, schemer);
}

} // namespace samos::config_manager
