#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <algorithm>
#include <vector>
#include <string>
#include "option_parser.hpp"

namespace samos_op = samos::user_interface::option_parser;

constexpr auto test_name = "TestOptionParser";
constexpr auto test_doc = "test option parser";

class TestOptionParser : public ::testing::Test {
public:
    int argc() const
    {
        return stored_args.size();
    }

    char** argv()
    {
        return stored_argv.data();
    }

    const std::vector<std::string>& nice_args() const
    {
        return stored_args;
    }

    void add_arg(std::string&& arg)
    {
        stored_args.emplace_back(std::move(arg));
        size_t size = stored_args.back().size();
        char* awful_string = new char[size + 1];

        std::copy(stored_args.back().begin(), stored_args.back().end(), awful_string);
        awful_string[size] = '\0';
        stored_argv.push_back(awful_string);
    }

    samos_op::OptionParser make_oparser()
    {
        return {argc(), argv(), test_name, test_doc};
    }

protected:
    TestOptionParser()
        :
        stored_args({test_name}),
        stored_argv(1)
    {
        stored_argv[0] = const_cast<char*>(stored_args.back().c_str());
    }

private:
    std::vector<std::string> stored_args;
    std::vector<char*> stored_argv;
};

TEST_F(TestOptionParser, AddFlag)
{
    samos_op::OptionParser oparser {make_oparser()};

    auto res = oparser.add_flag("v", "verbose", "doc string");

    EXPECT_TRUE(res.is_ok());
    res = oparser.add_flag("vv", "verbose", "doc string");
    EXPECT_TRUE(res.is_err());
    res = oparser.add_flag("", "verbose", "doc string");
    EXPECT_TRUE(res.is_err());

    /*
     *  EXPECT_TRUE(oparser.add_flag("は", "verbose", "doc string"));
     *  EXPECT_TRUE(oparser.add_flag("人", "verbose", "doc string"));
     *  EXPECT_TRUE(oparser.add_flag("す", "救い", "これはどくです"));
     *  EXPECT_TRUE(oparser.add_flag("す", "救い", "これはどくです"));
     *  EXPECT_FALSE(oparser.add_flag("じん", "救い", "これはどくです"));
     */
}

TEST_F(TestOptionParser, AddEmptyFlagSet)
{
    samos_op::OptionParser oparser {make_oparser()};

    EXPECT_TRUE(oparser.add_flag_set({}).is_ok());
}

TEST_F(TestOptionParser, AddBadFlagSet)
{
    struct StringSet
    {
        std::string flag;
        std::string opt;
        std::string doc;
    };

    auto test_bad_flag_set = [&](std::vector<StringSet>&& string_set)
    {
        samos_op::OptionParser oparser {make_oparser()};
        std::vector<samos_op::FlagSet> flag_set;
        bool first = true;
        auto make_flag_set = [&](StringSet& ss) -> samos_op::FlagSet {
            if (first)
            {
                first = false;
                return {ss.flag, ss.opt, ss.doc, oparser.create_help_callback()};
            }
            else
            {
                return {ss.flag, ss.opt, ss.doc, {}};
            }
        };

        std::transform(string_set.begin(), string_set.end(),
            std::back_inserter(flag_set),
            make_flag_set);
        return oparser.add_flag_set(std::move(flag_set));
    };

/*
 *  EXPECT_FALSE(test_bad_flag_set({{"", "help", "help string"}}));
 *  EXPECT_FALSE(test_bad_flag_set({{"h", "", "help string"}}));
 *  EXPECT_FALSE(test_bad_flag_set({{"h", "help", ""}}));
 *  EXPECT_FALSE(test_bad_flag_set({{"hh", "help", "help string"}}));
 *  EXPECT_FALSE(test_bad_flag_set({{"hh", "", "help string"}}));
 *  EXPECT_FALSE(test_bad_flag_set({{"h", "he lp", "help string"}}));
 *  EXPECT_FALSE(test_bad_flag_set({{"す", "救い", "これはどくです"}}));
 */
    /* FIXME: Enable this test when a UTF8 capable library is around. */
//     EXPECT_FALSE(test_bad_flag_set({{"l", "長い", "文書化"}}));
    EXPECT_TRUE(test_bad_flag_set({{"l", "long", "文書化"}}).is_ok());
}

TEST_F(TestOptionParser, AddGoodFlagSet)
{
    samos_op::OptionParser oparser {make_oparser()};

    auto res = oparser.add_flag_set({{"h", "help", "help string", oparser.create_help_callback()}});

    EXPECT_TRUE(res.is_ok());
}

TEST_F(TestOptionParser, ParseEmpty)
{
    samos_op::OptionParser oparser {make_oparser()};

    auto res = oparser.parse();

    EXPECT_TRUE(res.is_ok());
}

TEST_F(TestOptionParser, ParseNominal)
{
    add_arg("--helpify");
    add_arg("-H");
    add_arg("-V");
    add_arg("--verbosify");
    samos_op::OptionParser oparser {make_oparser()};
    int help_called = 0;
    std::string help_output;
    auto pre_callback = [&]()
    {
        help_called++;
        help_output = oparser.help();
    };
    int verbose_called = 0;
    auto verbose_callback = [&]()
    {
        verbose_called++;
    };

    EXPECT_TRUE(
        oparser.add_flag_set(
    {
        {"H", "helpify", "help doc", oparser.create_help_callback(pre_callback)},
        {"V", "verbosify", "verbose mode", verbose_callback}
    }).is_ok());

    ASSERT_TRUE(oparser.parse().is_ok());
    ASSERT_TRUE(oparser.handle_flag("helpify"));
    ASSERT_EQ(help_called, 1);
    EXPECT_THAT(help_output, testing::HasSubstr(test_name));
    EXPECT_THAT(help_output, testing::HasSubstr(test_doc));
    EXPECT_THAT(help_output, testing::HasSubstr("H"));
    EXPECT_THAT(help_output, testing::HasSubstr("helpify"));
    EXPECT_THAT(help_output, testing::HasSubstr("help doc"));
    EXPECT_THAT(help_output, testing::HasSubstr("V"));
    EXPECT_THAT(help_output, testing::HasSubstr("verbosify"));
    EXPECT_THAT(help_output, testing::HasSubstr("verbose mode"));

    help_output= "";
    ASSERT_TRUE(oparser.handle_flag("H"));
    ASSERT_EQ(help_called, 2);
    EXPECT_THAT(help_output, testing::HasSubstr(test_name));
    EXPECT_THAT(help_output, testing::HasSubstr(test_doc));
    EXPECT_THAT(help_output, testing::HasSubstr("H"));
    EXPECT_THAT(help_output, testing::HasSubstr("helpify"));
    EXPECT_THAT(help_output, testing::HasSubstr("help doc"));
    EXPECT_THAT(help_output, testing::HasSubstr("V"));
    EXPECT_THAT(help_output, testing::HasSubstr("verbosify"));
    EXPECT_THAT(help_output, testing::HasSubstr("verbose mode"));

    ASSERT_TRUE(oparser.handle_flag("verbosify"));
    ASSERT_EQ(verbose_called, 1);
    ASSERT_TRUE(oparser.handle_flag("V"));
    ASSERT_EQ(verbose_called, 2);
}

TEST_F(TestOptionParser, ParseBad)
{
    add_arg("--lollersk8s");
    samos_op::OptionParser oparser {make_oparser()};

    auto res = oparser.parse();

    EXPECT_TRUE(res.is_err());
}

TEST_F(TestOptionParser, HandleFlagBad)
{
    add_arg("--help");
    samos_op::OptionParser oparser {make_oparser()};
    int help_called = 0;
    auto pre_callback = [&]()
    {
        help_called++;
    };

    EXPECT_TRUE(
        oparser.add_flag_set(
    {
        {"h", "help", "help doc", oparser.create_help_callback(pre_callback)},
    }).is_ok());

    EXPECT_TRUE(oparser.parse().is_ok());
    EXPECT_FALSE(oparser.handle_flag("lollersc8s"));
    EXPECT_EQ(help_called, 0);
}

TEST_F(TestOptionParser, HandleFlagNominal)
{
    add_arg("--help");
    samos_op::OptionParser oparser {make_oparser()};
    int help_called = 0;
    auto pre_callback = [&]()
    {
        help_called++;
    };

    EXPECT_TRUE(
        oparser.add_flag_set(
    {
        {"h", "help", "help doc", oparser.create_help_callback(pre_callback)},
    }).is_ok());

    EXPECT_TRUE(oparser.parse().is_ok());
    EXPECT_TRUE(oparser.handle_flag("help"));
    EXPECT_EQ(help_called, 1);
}
