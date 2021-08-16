#include <gtest/gtest.h>
#include "metaforeas.hpp"

class TestMetaforeas : public ::testing::Test {
protected:
    TestMetaforeas() = default;
};

TEST_F(TestMetaforeas, BasicAssertions)
{
    ASSERT_TRUE(true);
}
