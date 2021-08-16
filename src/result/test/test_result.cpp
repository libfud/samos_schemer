#include "gtest/gtest.h"
#include "result.hpp"
#include <string>

namespace result = samos::result;
using GResult = result::Result<uint, std::string>;

TEST(ResultTest, OkResult)
{
    GResult ok_res = GResult::ok(55);

    ASSERT_TRUE(ok_res.is_ok());
    ASSERT_FALSE(ok_res.is_err());
    ASSERT_EQ(ok_res.get_ok(), 55);
    EXPECT_THROW({ok_res.get_err();}, std::bad_variant_access);
}

TEST(ResultTest, ErrResult)
{
    GResult err_res = GResult::err("Error");

    ASSERT_TRUE(err_res.is_err());
    ASSERT_FALSE(err_res.is_ok());
    auto err = err_res.get_err();

    ASSERT_STREQ(err.c_str(), "Error");
    EXPECT_THROW({err_res.get_ok();}, std::bad_variant_access);
}
