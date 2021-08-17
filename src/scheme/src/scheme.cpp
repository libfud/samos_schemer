#include "scheme.hpp"
#include "logger.hpp"

#include <cassert>
#include <string>

namespace samos::scheme {

std::string format_sexp_type(SexpType t)
{
    switch (t)
    {
    case SexpType::Unknown:
        return "Unknown";

    case SexpType::Boolean:
        return "Boolean";

    case SexpType::Integer:
        return "Integer";

    case SexpType::Flonum:
        return "Float";

    case SexpType::String:
        return "String";

    case SexpType::Symbol:
        return "Symbol";

    case SexpType::Pair:
        return "Pair";

    default:
        return "Unknown";
    }
}

Schemer::Schemer()
    :
    registered_c_types{}
{
    sexp_scheme_init();

    context = sexp_make_eval_context(NULL, NULL, NULL, 0, 0);

    environment = sexp_load_standard_env(context, NULL, SEXP_SEVEN);

    assert(!sexp_exceptionp(environment));

    sexp res = sexp_load_standard_ports(context, environment, stdin, stdout, stderr, 1);

    assert(!sexp_exceptionp(res));
}

Schemer::~Schemer()
{
    sexp_destroy_context(context);
}

sexp Schemer::eval(const std::string& input)
{
    return sexp_eval_string(context, input.c_str(), -1, environment);
}

void Schemer::load(const std::string& filename)
{
    auto obj1 = sexp_c_string(context, filename.c_str(), -1);

    (void)sexp_load(context, obj1, environment);
}

SchemerResult<sexp> Schemer::read_from_file(const std::string& filename)
{
    /* FIXME : create file path class to handle this properly */
    (void)filename;
    sexp file_contents = sexp_eval_string(
        context,
        fmt::format("(with-input-from-file \"{}\" read)", filename).c_str(),
        -1,
        environment);

    return SchemerResult<sexp>::ok(file_contents);
}

void Schemer::print_exception(const sexp& result)
{
    if (sexp_exceptionp(result))
    {
        sexp_gc_var1(out_port);
        sexp_gc_preserve1(context, out_port);
        /* FIXME: Reroute this into a string and handle printing elsewhere. */
        out_port = sexp_current_error_port(context);
        sexp_print_exception(context, result, out_port);
        sexp_gc_release1(context);
    }
}

std::string Schemer::sexp_to_string(const sexp& result, bool print_exception)
{
    if (print_exception)
    {
        this->print_exception(result);
    }

    return {sexp_string_data(sexp_write_to_string(context, result))};
}

SchemerResult<> Schemer::import_module(SchemeModule mod)
{
    sexp_gc_var4(mod_sexp, import_fn, import_statement, tmp);
    sexp_gc_preserve4(context, mod_sexp, import_fn, import_statement, tmp);

    import_fn = sexp_intern(context, "import", -1);
    scheme_module_to_sexp(mod, mod_sexp);

    import_statement = sexp_list2(context, import_fn, mod_sexp);

    auto import_string = fmt::format(
        "(with-exception-handler {} (lambda () {}))",
        "(lambda (exn) (display \"Whoopsie doodles\") #f)",
        sexp_to_string(import_statement)
    );

    tmp = sexp_eval_string(context, import_string.c_str(), -1, environment);
    if (sexp_exceptionp(tmp))
    {
        sexp_gc_release4(context);
        return SchemerResult<>::err(SchemeException{});
    }

    sexp_gc_release4(context);

    return SchemerResult<>::ok({});
}

bool Schemer::sexp_equal(sexp& a, sexp& b) const
{
    return sexp_equalp(context, a, b);
}

SexpType Schemer::sexp_type(sexp& obj) const
{
    SexpType t{SexpType::Unknown};

    if (sexp_booleanp(obj))
    {
        t = SexpType::Boolean;
    }
    else if (sexp_integerp(obj))
    {
        t = SexpType::Integer;
    }
    else if (sexp_flonump(obj))
    {
        t = SexpType::Flonum;
    }
    else if (sexp_stringp(obj))
    {
        t = SexpType::String;
    }
    else if (sexp_symbolp(obj))
    {
        t = SexpType::Symbol;
    }
    else if (sexp_pairp(obj))
    {
        t = SexpType::Pair;
    }

    return t;
}

SchemerResult<bool> Schemer::get_bool(sexp& obj) const
{
    return get_value<bool>(
        obj,
        [](sexp s) {return sexp_unbox_boolean(s);},
        SexpType::Boolean);
}

SchemerResult<sexp_sint_t> Schemer::get_fixnum(sexp& obj) const
{
    return get_value<sexp_sint_t>(
        obj,
        [](sexp s) {return sexp_unbox_fixnum(s);},
        SexpType::Boolean);
}

SchemerResult<int> Schemer::get_int(sexp& obj) const
{
    return get_value<int>(
        obj,
        [](sexp s) {return sexp_unbox_fixnum(s);},
        SexpType::Integer);
}

SchemerResult<double> Schemer::get_flonum(sexp& obj) const
{
    return get_value<double>(
        obj,
        [](sexp s) {return sexp_flonum_value(s);},
        SexpType::Flonum);
}

SchemerResult<std::string> Schemer::get_string(sexp& obj) const
{
    return get_value<std::string>(
        obj,
        [](sexp s) {return sexp_string_data(s);},
        SexpType::String);
}

SchemerResult<Symbol> Schemer::get_symbol(sexp& obj) const
{
    return get_value<Symbol>(
        obj,
        [&](sexp s) {return sexp_string_data(sexp_symbol_to_string(this->context, s));},
        SexpType::Symbol);
}

SchemerResult<SexpCppValue> Schemer::get_cpp_value(sexp& obj) const
{
    using SexpCppResult = SchemerResult<SexpCppValue>;
    auto stype = sexp_type(obj);

    auto result_rewrapper = [](auto res) -> SexpCppResult
    {
        if (res.is_ok())
        {
            // Schemer
            return SexpCppResult::ok(res.get_ok());
        }
        else
        {
            return SexpCppResult::err(res.get_err());
        }
    };

    switch (stype)
    {
    case SexpType::Unknown:
        log::logger::log(LogLevel::Info, "{}, CPP value: Unknown Type", __LINE__);
        return SexpCppResult::err(TypeNotFound{});

    case SexpType::Boolean:
        log::logger::log(LogLevel::Info, "{}, CPP value: Boolean Type", __LINE__);
        return result_rewrapper(get_bool({obj}));

    case SexpType::Integer:
        log::logger::log(LogLevel::Info, "{}, CPP value: Integer Type", __LINE__);
        return result_rewrapper(get_int({obj}));

    case SexpType::Flonum:
        log::logger::log(LogLevel::Info, "{}, CPP value: Flonum Type", __LINE__);
        return result_rewrapper(get_flonum({obj}));

    case SexpType::String:
        log::logger::log(LogLevel::Info, "{}, CPP value: String Type", __LINE__);
        return result_rewrapper(get_string({obj}));

    case SexpType::Symbol:
        log::logger::log(LogLevel::Info, "{}, CPP value: Symbol Type", __LINE__);
        return result_rewrapper(get_symbol({obj}));

    case SexpType::Pair:
        log::logger::log(LogLevel::Info, "{}, CPP value: Pair Type", __LINE__);
        return SexpCppResult::err(BadTypeError{});

    default:
        return SexpCppResult::err(BadTypeError{});
    }
}

SchemerResult<sexp> Schemer::car(sexp& obj) const
{
    return get_value<sexp>(
        obj,
        [&](sexp s) {return sexp_car(s);},
        SexpType::Pair);
}

SchemerResult<sexp> Schemer::cdr(sexp& obj) const
{
    return get_value<sexp>(
        obj,
        [&](sexp s) {return sexp_cdr(s);},
        SexpType::Pair);
}

SchemerResult<std::pair<sexp, sexp>> Schemer::car_cdr(sexp& obj) const
{
    return get_value<std::pair<sexp, sexp>>(
        obj,
        [&](sexp s) {return std::make_pair(sexp_car(s), sexp_cdr(s));},
        SexpType::Pair);
}

SchemerResult<sexp> Schemer::assq(const std::string& key, sexp& obj)
{
    using SchemerAssqResult = SchemerResult<sexp>;
    SexpType t = sexp_type(obj);

    if (t != SexpType::Pair)
    {
        return SchemerAssqResult::err(SexpTypeError{SexpType::Pair, t});
    }

    sexp_gc_var2(skey, alist);
    sexp_gc_preserve2(context, skey, alist);
    skey = sexp_intern(context, key.c_str(), -1);

    alist = sexp_assq(context, skey, obj);

    t = sexp_type(alist);
    if (t == SexpType::Boolean)
    {
        return SchemerAssqResult::err(AssocKeyNotFound{key});
    }
    if (t != SexpType::Pair)
    {
        return SchemerAssqResult::err(SexpTypeError{SexpType::Pair, t});
    }

    sexp contents = sexp_cdr(alist);

    sexp_gc_release1(context);

    return SchemerAssqResult::ok(contents);
}

SchemerResult<> Schemer::define_ffi_op_details(
    const std::string& op_name,
    sexp& ret_type,
    const std::vector<sexp>& arg_types,
    sexp& op
)
{
    size_t num_args = arg_types.size();

    if (!sexp_opcodep(op))
    {
        sexp output = sexp_write_to_string(context, op);
        log(LogLevel::Error, "Error registering op {}: {}", op_name, sexp_string_data(output));
        return SchemerResult<>::err(OpRegistrationError{});
    }

    sexp_opcode_return_type(op) = ret_type;

    if (num_args > 0)
    {
        sexp_opcode_arg1_type(op) = arg_types[0];
    }

    if (num_args > 1)
    {
        sexp_opcode_arg2_type(op) = arg_types[1];
    }

    if (num_args > 2)
    {
        sexp_opcode_arg3_type(op) = arg_types[2];
    }

    if (num_args > 3)
    {
        sexp_opcode_argn_type(op) = sexp_make_vector(
            context,
            sexp_make_fixnum(num_args - 3),
            sexp_make_fixnum(SEXP_OBJECT));
        for (size_t idx = 3; idx < num_args; ++idx)
        {
            sexp_vector_set(
                sexp_opcode_argn_type(op),
                sexp_make_fixnum(idx - 3),
                arg_types[idx]);
        }
    }

    log(LogLevel::Info, "Registered op {}", op_name);
    return SchemerResult<>::ok({});
}

void Schemer::scheme_module_to_sexp(SchemeModule mod, sexp& mod_sexp)
{
    if (std::holds_alternative<SrfiType>(mod))
    {
        return srfi_module_to_sexp(std::get<SrfiType>(mod), mod_sexp);
    }
    else
    {
        assert(std::holds_alternative<ChibiModule>(mod));
        return chibi_module_to_sexp(std::get<ChibiModule>(mod), mod_sexp);
    }
}

void Schemer::srfi_module_to_sexp(SrfiType mod, sexp& mod_sexp)
{
    sexp_gc_var1(srfi_str);
    sexp_gc_preserve1(context, srfi_str);
    srfi_str = sexp_intern(context, "srfi", -1);
    mod_sexp = sexp_list2(context, srfi_str, sexp_make_fixnum(static_cast<sexp_sint_t>(mod)));
    sexp_gc_release1(context);
}

void Schemer::chibi_module_to_sexp(ChibiModule mod, sexp& mod_sexp)
{
    const char* scope_str;
    const char* extension_str;
    bool extended_scope = false;

    switch (mod)
    {
    case ChibiModule::ChibiApp:
        scope_str = "app";
        break;

    case ChibiModule::ChibiAst:
        scope_str = "ast";
        break;

    case ChibiModule::ChibiBase64:
        scope_str = "base64";
        break;

    case ChibiModule::ChibiBytevector:
        scope_str = "bytevector";
        break;

    case ChibiModule::ChibiConfig:
        scope_str = "config";
        break;

    case ChibiModule::ChibiCryptoMd5:
        scope_str = "crypto";
        extension_str = "md5";
        extended_scope = true;
        break;

    case ChibiModule::ChibiCryptoRsa:
        scope_str = "crypto";
        extension_str = "rsa";
        extended_scope = true;
        break;

    case ChibiModule::ChibiCryptoSha2:
        scope_str = "crypto";
        extension_str = "sha2";
        extended_scope = true;
        break;

    case ChibiModule::ChibiDiff:
        scope_str = "diff";
        break;

    case ChibiModule::ChibiDisasm:
        scope_str = "disasm";
        break;

    case ChibiModule::ChibiDoc:
        scope_str = "doc";
        break;

    case ChibiModule::ChibiEditdistance:
        scope_str = "edit-distance";
        break;

    case ChibiModule::ChibiEquiv:
        scope_str = "equiv";
        break;

    case ChibiModule::ChibiFilesystem:
        scope_str = "filesystem";
        break;

    case ChibiModule::ChibiGeneric:
        scope_str = "generic";
        break;

    case ChibiModule::ChibiHeapStats:
        scope_str = "heap-stats";
        break;

    case ChibiModule::ChibiIo:
        scope_str = "io";
        break;

    case ChibiModule::ChibiIsetBase:
        scope_str = "iset";
        extension_str = "base";
        extended_scope = true;
        break;

    case ChibiModule::ChibiIsetConstructors:
        scope_str = "iset";
        extension_str = "constructors";
        extended_scope = true;
        break;

    case ChibiModule::ChibiIsetIterators:
        scope_str = "iset";
        extension_str = "iterators";
        extended_scope = true;
        break;

    case ChibiModule::ChibiJson:
        scope_str = "json";
        break;

    case ChibiModule::ChibiLoop:
        scope_str = "loop";
        break;

    case ChibiModule::ChibiMatch:
        scope_str = "match";
        break;

    case ChibiModule::ChibiMathPrime:
        scope_str = "math";
        extension_str = "prime";
        extended_scope = true;
        break;

    case ChibiModule::ChibiMemoize:
        scope_str = "memoize";
        break;

    case ChibiModule::ChibiMime:
        scope_str = "mime";
        break;

    case ChibiModule::ChibiModules:
        scope_str = "modules";
        break;

    case ChibiModule::ChibiNet:
        scope_str = "net";
        break;

    case ChibiModule::ChibiNetHttpServer:
        scope_str = "net";
        extension_str = "http-server";
        extended_scope = true;
        break;

    case ChibiModule::ChibiNetServlet:
        scope_str = "net";
        extension_str = "servlet";
        extended_scope = true;
        break;

    case ChibiModule::ChibiParse:
        scope_str = "parse";
        break;

    case ChibiModule::ChibiPathname:
        scope_str = "pathname";
        break;

    case ChibiModule::ChibiProcess:
        scope_str = "process";
        break;

    case ChibiModule::ChibiRepl:
        scope_str = "repl";
        break;

    case ChibiModule::ChibiScribble:
        scope_str = "scribble";
        break;

    case ChibiModule::ChibiString:
        scope_str = "string";
        break;

    case ChibiModule::ChibiStty:
        scope_str = "stty";
        break;

    case ChibiModule::ChibiSxml:
        scope_str = "sxml";
        break;

    case ChibiModule::ChibiSystem:
        scope_str = "system";
        break;

    case ChibiModule::ChibiTempFile:
        scope_str = "temp-file";
        break;

    case ChibiModule::ChibiTest:
        scope_str = "test";
        break;

    case ChibiModule::ChibiTime:
        scope_str = "time";
        break;

    case ChibiModule::ChibiTrace:
        scope_str = "trace";
        break;

    case ChibiModule::ChibiUri:
        scope_str = "uri";
        break;

    case ChibiModule::ChibiWeak:
        scope_str = "weak";
        break;
    }

    sexp_gc_var3(chibi_str, scope, extension);
    sexp_gc_preserve3(context, chibi_str, scope, extension);
    chibi_str = sexp_intern(context, "chibi", -1);
    mod_sexp = sexp_list2(context, chibi_str, sexp_make_fixnum(static_cast<sexp_sint_t>(mod)));

    scope = sexp_intern(context, scope_str, -1);

    if (!extended_scope)
    {
        mod_sexp = sexp_list2(context, chibi_str, scope);
    }
    else
    {
        sexp_gc_var1(temp);
        sexp_gc_preserve1(context, temp);
        extension = sexp_intern(context, extension_str, -1);
        temp = sexp_list2(context, scope, extension);
        mod_sexp = sexp_cons(context, chibi_str, temp);
        sexp_gc_release1(temp);
    }

    sexp_gc_release3(context);
}

} // namespace samos::scheme
