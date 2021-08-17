#ifndef SAMOS_KELYPHOS_HPP
#define SAMOS_KELYPHOS_HPP

#include "ed_line.hpp"
#include "scheme.hpp"

#include <chibi/eval.h>

extern "C" {
sexp sexp_ed_enable_history_stub(sexp ctx, sexp self, sexp_sint_t n, sexp arg0);
sexp sexp_ed_disable_history_stub(sexp ctx, sexp self, sexp_sint_t n, sexp arg0);
}

namespace samos::user_interface::kelyphos {

class Kelyphos {
public:

    Kelyphos() = delete;

    explicit Kelyphos(ed_line::EdLine* editor);

    ~Kelyphos();

    void repl();

private:

    void read();

    bool check_quit(const std::string& input);

    bool check_help(const std::string& input);

    void print(const sexp& result);

    ed_line::EdLine* editor;

    EdLinePOD ed_pod;

    scheme::Schemer schemer;
};

} // namespace samos::user_interface::kelyphos

#endif // SAMOS_KELYPHOS_HPP
