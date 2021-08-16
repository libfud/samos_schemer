#include "ed_line.hpp"
#include <fmt/core.h>

namespace samos::user_interface::ed_line {

constexpr const char* word_break_characters = " \t.,-%!;:=*~^'\"/?<>|[](){}";

EdLine::EdLine(std::string prompt)
    :
    editor(),
    prompt(prompt),
    history_path(),
    max_history_length(255),
    max_hint_rows(1),
    keywords(),
    history_enabled(false)
{
    // FIXME: Only relevant for windows.
    editor.install_window_change_handler();

    editor.set_max_history_size(max_history_length);

    editor.set_max_hint_rows(max_hint_rows);

    // FIXME set completion, highlighter, and hint callbacks
    editor.set_word_break_characters(word_break_characters);
    // FIXME set these to variables
    editor.set_completion_count_cutoff(128);
    editor.set_double_tab_completion(false);
    editor.set_complete_on_empty(false);
    editor.set_beep_on_ambiguous_completion(false);
    editor.set_no_color(true);

    // showcase key bindings
    editor.bind_key_internal(replxx::Replxx::KEY::BACKSPACE, "delete_character_left_of_cursor");
    editor.bind_key_internal(replxx::Replxx::KEY::DELETE, "delete_character_under_cursor");
    editor.bind_key_internal(replxx::Replxx::KEY::LEFT, "move_cursor_left");
    editor.bind_key_internal(replxx::Replxx::KEY::RIGHT, "move_cursor_right");
    editor.bind_key_internal(replxx::Replxx::KEY::UP, "history_previous");
    editor.bind_key_internal(replxx::Replxx::KEY::DOWN, "history_next");
    editor.bind_key_internal(replxx::Replxx::KEY::PAGE_UP, "history_first");
    editor.bind_key_internal(replxx::Replxx::KEY::PAGE_DOWN, "history_last");
    editor.bind_key_internal(replxx::Replxx::KEY::HOME, "move_cursor_to_begining_of_line");
    editor.bind_key_internal(replxx::Replxx::KEY::END, "move_cursor_to_end_of_line");
    editor.bind_key_internal(replxx::Replxx::KEY::TAB, "complete_line");
    editor.bind_key_internal(replxx::Replxx::KEY::control(replxx::Replxx::KEY::LEFT), "move_cursor_one_word_left");
    editor.bind_key_internal(replxx::Replxx::KEY::control(replxx::Replxx::KEY::RIGHT), "move_cursor_one_word_right");
    editor.bind_key_internal(replxx::Replxx::KEY::control(replxx::Replxx::KEY::UP), "hint_previous");
    editor.bind_key_internal(replxx::Replxx::KEY::control(replxx::Replxx::KEY::DOWN), "hint_next");
    editor.bind_key_internal(replxx::Replxx::KEY::control(replxx::Replxx::KEY::ENTER), "commit_line");
    editor.bind_key_internal(replxx::Replxx::KEY::control('R'), "history_incremental_search");
    editor.bind_key_internal(replxx::Replxx::KEY::control('W'), "kill_to_begining_of_word");
    editor.bind_key_internal(replxx::Replxx::KEY::control('U'), "kill_to_begining_of_line");
    editor.bind_key_internal(replxx::Replxx::KEY::control('K'), "kill_to_end_of_line");
    editor.bind_key_internal(replxx::Replxx::KEY::control('Y'), "yank");
    editor.bind_key_internal(replxx::Replxx::KEY::control('L'), "clear_screen");
    editor.bind_key_internal(replxx::Replxx::KEY::control('D'), "send_eof");
    editor.bind_key_internal(replxx::Replxx::KEY::control('C'), "abort_line");
    editor.bind_key_internal(replxx::Replxx::KEY::control('T'), "transpose_characters");
    editor.bind_key_internal(replxx::Replxx::KEY::meta(replxx::Replxx::KEY::BACKSPACE), "kill_to_whitespace_on_left");
    editor.bind_key_internal(replxx::Replxx::KEY::meta('p'), "history_common_prefix_search");
    editor.bind_key_internal(replxx::Replxx::KEY::meta('n'), "history_common_prefix_search");
    editor.bind_key_internal(replxx::Replxx::KEY::meta('d'), "kill_to_end_of_word");
    editor.bind_key_internal(replxx::Replxx::KEY::meta('y'), "yank_cycle");
    editor.bind_key_internal(replxx::Replxx::KEY::meta('u'), "uppercase_word");
    editor.bind_key_internal(replxx::Replxx::KEY::meta('l'), "lowercase_word");
    editor.bind_key_internal(replxx::Replxx::KEY::meta('c'), "capitalize_word");
}

EdLine::~EdLine()
{
}

std::string EdLine::read()
{
    return get_input().value_or("\0");
}

void EdLine::print(std::string& input)
{
    editor.print("%s\n", input.c_str());
}

void EdLine::add_to_history(std::string& input)
{
    editor.history_add(input);
}

void EdLine::set_history_file(std::string& path)
{
    history_path = path;
}

void EdLine::load_history()
{
    bool result = editor.history_load(history_path);

    // FIXME

    (void)result;
}

void EdLine::save_history()
{
    bool result = editor.history_save(history_path);

    // FIXME

    (void)result;
}

void EdLine::enable_history()
{
    fmt::print("Enabled history\n");
    history_enabled = true;
}

void EdLine::disable_history()
{
    fmt::print("Disabled history\n");
    history_enabled = false;
}

std::optional<std::string> EdLine::get_input()
{
    const char* cinput {nullptr};

    do
    {
        cinput = editor.input(prompt);
    } while ((nullptr == cinput) && (EAGAIN == errno));

    if (cinput == nullptr)
    {
        return {};
    }
    else if (history_enabled)
    {
        editor.history_add(cinput);
    }
    ;

    return {cinput};
}

} // namespace samos::user_interface::ed_line

void ed_enable_history(EdLinePOD editor)
{
    if (editor.editor == nullptr)
    {
        fmt::print(stderr, "null pointer\n");
    }
    else
    {
        editor.editor->enable_history();
    }
}

void ed_disable_history(EdLinePOD editor)
{
    if (editor.editor == nullptr)
    {
        fmt::print(stderr, "null pointer\n");
    }
    else
    {
        editor.editor->disable_history();
    }
}
