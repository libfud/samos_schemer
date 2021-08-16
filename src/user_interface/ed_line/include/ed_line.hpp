#ifndef SAMOS_ED_LINE_HPP
#define SAMOS_ED_LINE_HPP

#include <string>
#include <cstdint>
#include <optional>
#include "replxx.hxx"

namespace samos::user_interface::ed_line {

/*
 * constexpr std::vector<std::string> keywords = {
 *  ",help",
 *  ",quit",
 *  ",exit"
 * };
 */

class EdLine {
public:

    EdLine() = delete;

    explicit EdLine(std::string prompt);

    ~EdLine();

    virtual std::string read();

    virtual void print(std::string& input);

    void add_to_history(std::string& input);

    void set_history_file(std::string& path);

    void load_history();

    void save_history();

    void enable_history();

    void disable_history();

private:

    std::optional<std::string> get_input();

    replxx::Replxx editor;

    std::string prompt;

    std::string history_path;

    uint16_t max_history_length;

    uint8_t max_hint_rows;

    std::vector<std::string> keywords;

    bool history_enabled;
};

} // namespace samos::user_interface::ed_line

extern "C"
{
struct EdLinePOD
{
    samos::user_interface::ed_line::EdLine* editor;
};

void ed_enable_history(EdLinePOD pod_ed);
void ed_disable_history(EdLinePOD pod_ed);
}



#endif // SAMOS_ED_LINE_HPP
