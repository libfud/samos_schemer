#include "gtest/gtest.h"
#include "ed_line.hpp"

namespace ed_line = samos::user_interface::ed_line;

TEST(EdLineTest, Create)
{
    ed_line::EdLine("This is a prompt");
}
