#include "result.hpp"
#include "scheme.hpp"

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>

namespace samos::config_manager
{

class PropertyAlreadyRegistered
{
public:
    std::string format()
    {
        return {"Property already registered"};
    }
};

class PropertyNotRegistered
{
public:
    std::string format()
    {
        return {"Property not registered"};
    }
};

class NotConfigMap
{
public:
    explicit NotConfigMap(sexp value) : value{value}
    {
    }

    std::string format()
    {
        return {"Not a config map"};
    }

private:
    sexp value;
};

class TypeError
{
public:
    std::string format()
    {
        return {"Type Error"};
    }
};

namespace detail
{

using CfgErrVar = std::variant<
    PropertyAlreadyRegistered,
    PropertyNotRegistered,
    NotConfigMap,
    TypeError,
    scheme::SchemerError>;

}

class ConfigError : public detail::CfgErrVar
{
    using detail::CfgErrVar::variant;
public:
    std::string format()
    {
        if (std::holds_alternative<PropertyAlreadyRegistered>(*this))
        {
            return std::get<PropertyAlreadyRegistered>(*this).format();
        }
        else if (std::holds_alternative<PropertyNotRegistered>(*this))
        {
            return std::get<PropertyNotRegistered>(*this).format();
        }
        else if (std::holds_alternative<NotConfigMap>(*this))
        {
            return std::get<NotConfigMap>(*this).format();
        }
        else if (std::holds_alternative<TypeError>(*this))
        {
            return std::get<TypeError>(*this).format();
        }
        else
        {
            assert(std::holds_alternative<scheme::SchemerError>(*this));
            return std::get<scheme::SchemerError>(*this).format();
        }
    }
};

template <typename T = std::monostate>
using ConfigResult = result::Result<T, ConfigError>;

class ConfigMap;

using ConfigValue = std::variant<scheme::SexpCppValue, ConfigMap*>;

class ConfigMap {
public:
    ConfigMap();
    ~ConfigMap();
    ConfigMap(const ConfigMap& rhs);
    ConfigMap(ConfigMap&& rhs);
    ConfigMap& operator=(const ConfigMap& rhs);
    ConfigMap& operator=(ConfigMap&& rhs);

    [[nodiscard]] ConfigResult<> register_property(std::string&& key, ConfigValue&& value);

    [[nodiscard]] ConfigResult<> register_properties(
        std::vector<std::pair<std::string, ConfigValue>>&& key_value_pairs);

    [[nodiscard]] ConfigResult<> set_property(const std::string& key, ConfigValue&& value);

    [[nodiscard]] ConfigResult<> load_from_sexp(sexp& sexp_config, scheme::Schemer& schemer);

    template <typename S>
    [[nodiscard]] bool has_property(S&& ident)
    {
        return config_map.contains(std::forward<S>(ident));
    }

    [[nodiscard]] ConfigResult<ConfigValue> pop_property(std::string&& key);

    [[nodiscard]] ConfigResult<ConfigValue> copy_property(const std::string& key) const;

    [[nodiscard]] ConfigResult<const ConfigValue*> ref_property(const std::string& key) const;

    [[nodiscard]] ConfigResult<bool> property_is_nested_map(const std::string& key) const;

private:

    std::unordered_map<std::string, ConfigValue> config_map;
    std::set<std::string> config_keys;
};

class ConfigNest {
public:
    ConfigNest();
    template <typename S>
    ConfigNest(S&& name, ConfigMap map)
        :
        name{name},
        default_map{map},
        loaded_map{map}
    {
    }

    [[nodiscard]] const std::string& get_name() const;

    [[nodiscard]] ConfigResult<ConfigMap> load_config_from_sexp(sexp& config_alist, scheme::Schemer& schemer);

private:
    std::string name;

    ConfigMap default_map;

    ConfigMap loaded_map;
};

class ConfigManager {
public:
    ConfigManager();

    explicit ConfigManager(ConfigNest root)
        :
        root_map{root},
        schemer{}
    {
    }

    [[nodiscard]] ConfigResult<ConfigMap> load_config_from_file(const std::string& filename);

private:
    ConfigNest root_map;

    scheme::Schemer schemer;
};

} // namespace samos::config_manager
