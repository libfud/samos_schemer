#include "lib_samos.hpp"
#include "ed_line.hpp"
#include "kelyphos.hpp"

void run_kelyphos()
{
    samos::user_interface::ed_line::EdLine editor("kelyphos> ");
    samos::user_interface::kelyphos::Kelyphos shell(&editor);

    shell.repl();
}
