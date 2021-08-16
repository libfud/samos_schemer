#ifndef SAMOS_SCHEME_HPP
#define SAMOS_SCHEME_HPP

#include "scheme_module.hpp"
#include "logger.hpp"
#include "result.hpp"

#include <chibi/eval.h>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace samos::scheme {

using log::logger::log;
using log::logger::LogLevel;

enum class SexpType
{
    Unknown,
    Boolean,
    Integer,
    Flonum,
    String,
    Symbol,
    Pair,
};

std::string format_sexp_type(SexpType t);

class Symbol : public std::string
{
    using std::string::string;
};

namespace detail
{

using SexpCVVar = std::variant<
    bool,
    int,
    double,
    std::string,
    Symbol>;

};

class SexpCppValue : public detail::SexpCVVar
{
    using detail::SexpCVVar::variant;
};

class OpRegistrationError
{
public:
    std::string format()
    {
        return {"Error registering op"};
    }
};

class TypeAlreadyRegistered
{
public:
    std::string format()
    {
        return {"Type already registered"};
    }
};

class TypeRegistrationError
{
public:
    std::string format()
    {
        return {"Type registration error"};
    }
};

class TypeNotFound
{
public:
    std::string format()
    {
        return {"Type not found"};
    }
};

class BadTypeError
{
public:
    std::string format()
    {
        return {"Bad type for operation"};
    }
};

class SexpTypeError
{
public:
    SexpTypeError(SexpType expected, SexpType got)
        :
        expected{expected},
        got{got}
    {
    }

    std::string format()
    {
        return fmt::format(
            "Bad type: expected {}, got {} for operation",
            format_sexp_type(expected),
            format_sexp_type(got));
    }

private:
    SexpType expected;
    SexpType got;
};

class FilenameError
{
public:
    std::string format()
    {
        return {"filename error"};
    }
};

class AssocKeyNotFound
{
public:
    explicit AssocKeyNotFound(const std::string& key) : key{key}
    {
    }

    std::string format()
    {
        return fmt::format("Key {} not found", key);
    }

private:
    std::string key;
};

class SchemeException {
    std::string format()
    {
        return "Exception";
    }
};

namespace detail {

using SchemeErrVariant = std::variant<
    OpRegistrationError,
    TypeAlreadyRegistered,
    TypeNotFound,
    SexpTypeError,
    FilenameError,
    BadTypeError,
    AssocKeyNotFound,
    SchemeException>;
}

class SchemerError : public detail::SchemeErrVariant
{
    using detail::SchemeErrVariant::variant;
public:
    std::string format()
    {
        if (std::holds_alternative<OpRegistrationError>(*this))
        {
            return std::get<OpRegistrationError>(*this).format();
        }
        else if (std::holds_alternative<TypeAlreadyRegistered>(*this))
        {
            return std::get<TypeAlreadyRegistered>(*this).format();
        }
        else if (std::holds_alternative<TypeNotFound>(*this))
        {
            return std::get<TypeNotFound>(*this).format();
        }
        else if (std::holds_alternative<SexpTypeError>(*this))
        {
            return std::get<SexpTypeError>(*this).format();
        }
        else if (std::holds_alternative<FilenameError>(*this))
        {
            return std::get<FilenameError>(*this).format();
        }
        else if (std::holds_alternative<BadTypeError>(*this))
        {
            return std::get<BadTypeError>(*this).format();
        }
        else
        {
            assert(std::holds_alternative<AssocKeyNotFound>(*this));
            return std::get<AssocKeyNotFound>(*this).format();
        }
    }
};

template <typename T = std::monostate>
using SchemerResult = result::Result<T, SchemerError>;

class Schemer {
public:

    Schemer();

    ~Schemer();

    template <typename F>
    SchemerResult<> define_ffi_op(
        std::string&& op_name,
        sexp&& ret_type,
        std::vector<sexp>&& arg_types,
        F stub)
    {
        size_t num_args = arg_types.size();

        sexp_gc_var3(name, tmp, op);
        sexp_gc_preserve3(context, name, tmp, op);

        op = sexp_define_foreign(context, environment, op_name.c_str(), num_args, stub);

        auto res = define_ffi_op_details(op_name, ret_type, arg_types, op);

        sexp_gc_release3(context);
        return res;
    }

    template <typename T>
    SchemerResult<> bind_symbol_to_c_object(std::string&& symbol, T* object)
    {
        sexp ptr;
        sexp sym;
        sexp_uint_t c_type;
        auto& c_type_info = typeid(T);
        size_t type_hash = c_type_info.hash_code();
        std::string type_name = c_type_info.name();

        if (registered_c_types.contains(type_hash))
        {
            c_type = registered_c_types.at(type_hash);
        }
        else
        {
            return SchemerResult<>::err(TypeNotFound{});
        }

        // NOTE: parent=NULL and freep=false may not always be the case
        ptr = sexp_make_cpointer(context, c_type, object, NULL, false);
        sym = sexp_intern(context, symbol.c_str(), -1);

        sexp_env_define(context, environment, sym, ptr);

        log(LogLevel::Info, "Bound symbol: {} to pointer of type {}", symbol, type_name);

        return SchemerResult<>::ok({});
    }

    template <typename T>
    SchemerResult<sexp> register_c_type()
    {
        static_assert(std::is_standard_layout<T>::value);

        auto& c_type_info = typeid(T);
        size_t type_hash = c_type_info.hash_code();
        std::string type_name = c_type_info.name();

        if (registered_c_types.contains(type_hash))
        {
            return SchemerResult<sexp>::err(TypeAlreadyRegistered{});
        }

        sexp sexp_POD_type_obj;

        sexp_gc_var2(name, tmp);

        sexp_gc_preserve2(context, name, tmp);

        name = sexp_c_string(context, type_name.c_str(), -1);
        sexp_POD_type_obj = sexp_register_c_type(context, name, sexp_finalize_c_type);
        tmp = sexp_string_to_symbol(context, name);
        sexp_env_define(context, environment, tmp, sexp_POD_type_obj);

        registered_c_types[type_hash] = sexp_type_tag(sexp_POD_type_obj);

        sexp_gc_release2(context);

        return SchemerResult<sexp>::ok(sexp_POD_type_obj);
    }

    sexp eval(const std::string& input);

    void load(const std::string& filename);

    SchemerResult<sexp> read_from_file(const std::string& filename);

    void print_exception(const sexp& result);

    std::string sexp_to_string(const sexp& result, bool print_exception = false);

    SchemerResult<> import_module(SchemeModule mod);

    bool sexp_equal(sexp& a, sexp& b) const;

    SexpType sexp_type(sexp& obj) const;

    SchemerResult<bool> get_bool(sexp& obj) const;

    SchemerResult<sexp_sint_t> get_fixnum(sexp& obj) const;

    SchemerResult<int> get_int(sexp& obj) const;

    SchemerResult<double> get_flonum(sexp& obj) const;

    SchemerResult<std::string> get_string(sexp& obj) const;

    SchemerResult<Symbol> get_symbol(sexp& obj) const;

    SchemerResult<SexpCppValue> get_cpp_value(sexp& obj) const;

    SchemerResult<sexp> car(sexp& obj) const;

    SchemerResult<sexp> cdr(sexp& obj) const;

    SchemerResult<std::pair<sexp, sexp>> car_cdr(sexp& obj) const;

    SchemerResult<sexp> assq(const std::string& key, sexp& obj);

private:

    template<typename T>
    SchemerResult<T> get_value(
        sexp obj,
        std::function<T(sexp)> constructor,
        SexpType expected) const
    {
        SexpType t = sexp_type(obj);

        if (t == expected)
        {
            return SchemerResult<T>::ok(constructor(obj));
        }
        else
        {
            return SchemerResult<T>::err(SexpTypeError{expected, t});
        }
    }

    [[nodiscard]] SchemerResult<> define_ffi_op_details(
        const std::string& op_name,
        sexp& ret_type,
        const std::vector<sexp>& arg_types,
        sexp& op);

    void scheme_module_to_sexp(SchemeModule mod, sexp& mod_sexp);

    void srfi_module_to_sexp(SrfiType mod, sexp& mod_sexp);

    void chibi_module_to_sexp(ChibiModule mod, sexp& mod_sexp);

    sexp context;

    sexp stack;

    sexp environment;

    std::unordered_map<size_t, sexp_uint_t> registered_c_types;
};

} // namespace samos::scheme

#endif // SAMOS_SCHEME_HPP
