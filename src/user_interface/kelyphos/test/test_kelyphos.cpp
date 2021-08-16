#include "gtest/gtest.h"
#include "kelyphos.hpp"
#include <vector>

namespace samos_ed = samos::user_interface::ed_line;
namespace kelyphos = samos::user_interface::kelyphos;

class TestEdLine : public samos_ed::EdLine
{
public:
    explicit TestEdLine(const std::string&& prompt)
        :
        EdLine(std::move(prompt)),
        in_buffer({}),
        out_buffer({})
    {
    }

    void push(std::string&& input)
    {
        in_buffer.emplace_back(std::move(input));
    }

    std::string pop()
    {
        std::string out;

        if (out_buffer.size() > 0)
        {
            out = out_buffer.back();
            out_buffer.pop_back();
        }
        else
        {
            out = "";
        }
        return out;
    }

    void print(std::string& input) override
    {
        samos_ed::EdLine::print(input);
        out_buffer.push_back(input);
    }

    std::string read() override
    {
        if (in_buffer.size() > 0)
        {
            auto val = in_buffer.back();
            in_buffer.pop_back();
            return val;
        }
        else
        {
            return {",quit"};
        }
    }

    size_t out_size()
    {
        return out_buffer.size();
    }

private:
    std::vector<std::string> in_buffer;
    std::vector<std::string> out_buffer;
};

TEST(TestKelyphos, CheckQuit)
{
    TestEdLine editor("test ");
    kelyphos::Kelyphos shell(&editor);

    shell.repl();
    EXPECT_EQ(editor.out_size(), 0);
}


TEST(TestKelyphos, CheckHelp)
{
    // FIXME
    TestEdLine editor("test ");
    kelyphos::Kelyphos shell(&editor);

    editor.push(",exit");
    editor.push(",help");
    editor.push("fake-help");
    editor.push("(foo)");
    editor.push("(bar 2)");
    editor.push("(ed_enable_history editor-pointer)");
    editor.push("(ed_disable_history editor-pointer)");

    shell.repl();

    EXPECT_EQ(editor.out_size(), 5);
}
