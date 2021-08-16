#include "kelyphos.hpp"
#include "logger.hpp"

#include <chibi/sexp.h>
#include <fmt/core.h>
#include <string>

int foo()
{
    return 42;
}

sexp sexp_foo_stub(sexp ctx, sexp self, sexp_sint_t n)
{
    sexp res;

    (void)self;
    (void)n;

    res = sexp_make_integer(ctx, foo());
    return res;
}

int bar(int x)
{
    return x * 2;
}

sexp sexp_bar_stub(sexp ctx, sexp self, sexp_sint_t n, sexp arg0)
{
    sexp res;

    (void)n;

    if (!sexp_exact_integerp(arg0))
    {
        return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg0);
    }
    res = sexp_make_integer(ctx, bar(sexp_sint_value(arg0)));
    return res;
}

sexp sexp_ed_enable_history_stub(sexp ctx, sexp self, sexp_sint_t n, sexp arg0)
{
    sexp res;

    (void)n;

    if (!(sexp_pointerp(arg0) && (sexp_pointer_tag(arg0) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    {
        return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), arg0);
    }

    res = (ed_enable_history(*static_cast<EdLinePOD*>(sexp_cpointer_value(arg0))),
        SEXP_VOID);

    return res;
}

sexp sexp_ed_disable_history_stub(sexp ctx, sexp self, sexp_sint_t n, sexp arg0)
{
    sexp res;

    (void)n;

    if (!(sexp_pointerp(arg0) && (sexp_pointer_tag(arg0) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    {
        return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), arg0);
    }

    res = (ed_disable_history(*static_cast<EdLinePOD*>(sexp_cpointer_value(arg0))),
        SEXP_VOID);

    return res;
}

namespace samos::user_interface::kelyphos {

Kelyphos::Kelyphos(ed_line::EdLine* editor)
    :
    editor(editor),
    ed_pod{editor},
    schemer{}
{
    // FIXME CHECK RESULTS
    auto sexp_res = schemer.register_c_type<EdLinePOD>();
    sexp sexp_EdLinePOD_type;

    if (sexp_res.is_ok())
    {
        sexp_EdLinePOD_type = sexp_res.get_ok();
    }
    else
    {
        throw "EdLine already registered.";
    }

    auto res = schemer.bind_symbol_to_c_object("editor-pointer", &ed_pod);

    if (res.is_err())
    {
        // time to make this private and go for a static constructor.
        throw "Failed to register type in Kelyphos!";
    }

    res = schemer.define_ffi_op(
        "foo",
        sexp_make_fixnum(SEXP_FIXNUM),
        {},
        sexp_foo_stub
    );

    res = schemer.define_ffi_op(
        "bar",
        sexp_make_fixnum(SEXP_FIXNUM),
        {sexp_make_fixnum(SEXP_FIXNUM)},
        sexp_bar_stub
    );
    res = schemer.define_ffi_op(
        "ed-enable-history",
        SEXP_VOID,
        {sexp_make_fixnum(sexp_type_tag(sexp_EdLinePOD_type))},
        sexp_ed_enable_history_stub
    );
    res = schemer.define_ffi_op(
        "ed-disable-history",
        SEXP_VOID,
        {sexp_make_fixnum(sexp_type_tag(sexp_EdLinePOD_type))},
        sexp_ed_disable_history_stub
    );
}

Kelyphos::~Kelyphos()
{
}

void Kelyphos::repl()
{
    while (true)
    {
        std::string input = editor->read();

        if (input.empty() || check_help(input))
        {
            continue;
        }
        else if (check_quit(input))
        {
            break;
        }
        else
        {
            auto output = schemer.eval(input);
            print(output);
            continue;
        }
    }
}

bool Kelyphos::check_quit(const std::string& input)
{
    auto quit_check = (
        (input.compare(0, 5, ",quit") == 0) ||
        (input.compare(0, 5, ",exit") == 0));

    if (quit_check)
    {
        // fixme add to history
    }

    return quit_check;
}

bool Kelyphos::check_help(const std::string& input)
{
    auto help_check = (input.compare(0, 5, ",help") == 0);

    if (help_check)
    {
        fmt::print(
            "{}\n\t{}\n{}\n\t{}\n{}\n\t{}\n",
            ",help",
            "displays the help ooutput",
            ",quit",
            "exit the repl",
            ",exit",
            "exit the repl");
    }

    return help_check;
}

void Kelyphos::print(const sexp& result)
{
    std::string output{schemer.sexp_to_string(result)};

    editor->print(output);
}

}
// namespace samos::user_interface::kelyphos
