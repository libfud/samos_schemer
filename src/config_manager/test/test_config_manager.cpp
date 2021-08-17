#include "config_manager.hpp"
#include "logger.hpp"

#include <gtest/gtest.h>

namespace samos::config_manager
{

using scheme::SexpCppValue;
using log::logger::log;
using log::logger::LogLevel;
using log::logger::set_level;

TEST(TestConfigMap, TestRegisterProperty)
{
    samos::log::logger::set_level(samos::log::logger::LogLevel::Debug);
    ConfigMap root_map;

    auto register_res = root_map.register_property("test", ConfigValue{1});

    ASSERT_TRUE(register_res.is_ok());

    ASSERT_TRUE(root_map.has_property("test"));

    register_res = root_map.register_property("test", ConfigValue{1});

    ASSERT_TRUE(register_res.is_err());

    auto test_value_res = root_map.pop_property("test");

    ASSERT_FALSE(root_map.has_property("test"));
    ASSERT_TRUE(test_value_res.is_ok());
    ConfigValue test_value = test_value_res.get_ok();

    ASSERT_TRUE(std::holds_alternative<SexpCppValue>(test_value));
    auto v = std::get<SexpCppValue>(test_value);

    ASSERT_TRUE(std::holds_alternative<int>(v));
    ASSERT_EQ(std::get<int>(v), 1);

    register_res = root_map.register_property("test", ConfigValue{1});
    ASSERT_TRUE(register_res.is_ok());
    ConfigMap new_root{std::move(root_map)};

    ASSERT_TRUE(new_root.has_property("test"));
    test_value_res = new_root.pop_property("test");
    ASSERT_FALSE(new_root.has_property("test"));
    ASSERT_TRUE(test_value_res.is_ok());
    test_value = test_value_res.get_ok();
    ASSERT_TRUE(std::holds_alternative<SexpCppValue>(test_value));
    v = std::get<SexpCppValue>(test_value);
    ASSERT_TRUE(std::holds_alternative<int>(v));
    ASSERT_EQ(std::get<int>(v), 1);

    register_res = new_root.register_property("test", ConfigValue{1});
    ASSERT_TRUE(register_res.is_ok());
    ConfigMap new_root2;

    new_root2= std::move(new_root);

    ASSERT_TRUE(new_root2.has_property("test"));
    test_value_res = new_root2.pop_property("test");
    ASSERT_FALSE(new_root2.has_property("test"));
    ASSERT_TRUE(test_value_res.is_ok());
    test_value = test_value_res.get_ok();
    ASSERT_TRUE(std::holds_alternative<SexpCppValue>(test_value));
    v = std::get<SexpCppValue>(test_value);
    ASSERT_TRUE(std::holds_alternative<int>(v));
    ASSERT_EQ(std::get<int>(v), 1);

    register_res = new_root2.register_property("test", ConfigValue{1});
    ASSERT_TRUE(register_res.is_ok());
    ConfigMap new_root3;

    new_root3 = new_root2;

    ASSERT_TRUE(new_root3.has_property("test"));
    test_value_res = new_root3.pop_property("test");
    ASSERT_FALSE(new_root3.has_property("test"));
    ASSERT_TRUE(test_value_res.is_ok());
    test_value = test_value_res.get_ok();
    ASSERT_TRUE(std::holds_alternative<SexpCppValue>(test_value));
    v = std::get<SexpCppValue>(test_value);
    ASSERT_TRUE(std::holds_alternative<int>(v));
    ASSERT_EQ(std::get<int>(v), 1);
}

TEST(TestConfigMap, TestSetProperty)
{
    ConfigMap root_map;

    std::string prop_key = "test";
    auto set_res = root_map.set_property(prop_key, ConfigValue{false});

    ASSERT_TRUE(set_res.is_err());
    ASSERT_FALSE(root_map.has_property("test"));

    auto register_res = root_map.register_property("test", ConfigValue{1});

    ASSERT_TRUE(register_res.is_ok());
    ASSERT_TRUE(root_map.has_property("test"));

    set_res = root_map.set_property(prop_key, ConfigValue{false});
    ASSERT_TRUE(set_res.is_ok());

    auto test_value_res = root_map.pop_property("test");

    ASSERT_FALSE(root_map.has_property("test"));
    ASSERT_TRUE(test_value_res.is_ok());
    ConfigValue test_value = test_value_res.get_ok();

    ASSERT_TRUE(std::holds_alternative<SexpCppValue>(test_value));
    auto v = std::get<SexpCppValue>(test_value);

    ASSERT_FALSE(std::holds_alternative<int>(v));
    ASSERT_TRUE(std::holds_alternative<bool>(v));
    ASSERT_EQ(std::get<bool>(v), false);
}

TEST(TestConfigMap, TestRegisterPropertyFail)
{
    ConfigMap root_map;

    auto register_res = root_map.register_property("test", ConfigValue{1});

    ASSERT_TRUE(register_res.is_ok());
    register_res = root_map.register_property("test", ConfigValue{1});
    ASSERT_TRUE(register_res.is_err());
}

TEST(TestConfigMap, TestRegisterProperties)
{
    ConfigMap root_map;
    ConfigMap root_map2;

    std::vector<std::pair<std::string, ConfigValue>> props {
        {"test1", ConfigValue{false}},
        {"test2", ConfigValue{false}},
    };
    auto props_copy1{props};
    auto props_copy2{props};

    props_copy2.emplace_back("test1", ConfigValue{1});
    auto register_res = root_map.register_properties(std::move(props_copy1));

    ASSERT_TRUE(register_res.is_ok());
    register_res = root_map2.register_properties(std::move(props_copy2));
    ASSERT_TRUE(register_res.is_err());
}

TEST(TestConfigMap, TestPopPropertyFail)
{
    ConfigMap root_map;

    ASSERT_FALSE(root_map.has_property("test"));
    auto test_value_res = root_map.pop_property("test");

    ASSERT_TRUE(test_value_res.is_err());
}

TEST(TestConfigMap, TestNestedHeap)
{
    ConfigMap root_map;
    ConfigMap* nested_1 = new ConfigMap;
    ConfigMap* nested_2 = new ConfigMap;

    auto register_res = nested_1->register_property("n1_prop", ConfigValue{true});

    ASSERT_TRUE(register_res.is_ok());
    ASSERT_TRUE(nested_1->has_property("n1_prop"));

    register_res = nested_2->register_property("n2_prop", ConfigValue{2});
    ASSERT_TRUE(register_res.is_ok());
    ASSERT_TRUE(nested_2->has_property("n2_prop"));

    register_res = root_map.register_property("root_prop", ConfigValue{3.0});
    ASSERT_TRUE(root_map.has_property("root_prop"));

    register_res = nested_1->register_property("nested_2", ConfigValue{nested_2});
    ASSERT_TRUE(register_res.is_ok());

    register_res = root_map.register_property("nested_1", nested_1);
    ASSERT_TRUE(register_res.is_ok());

    ASSERT_TRUE(root_map.has_property("nested_1"));
    ASSERT_TRUE(root_map.has_property("root_prop"));

    auto pop_res = root_map.pop_property("root_prop");

    ASSERT_TRUE(pop_res.is_ok());

    auto popped = pop_res.get_ok();

    ASSERT_TRUE(std::holds_alternative<SexpCppValue>(popped));
    ASSERT_TRUE(std::holds_alternative<double>(std::get<SexpCppValue>(popped)));
    ASSERT_EQ(std::get<double>(std::get<SexpCppValue>(popped)), 3.0);

    pop_res = root_map.pop_property("nested_1");
    ASSERT_TRUE(pop_res.is_ok());
    popped = pop_res.get_ok();
    ASSERT_TRUE(std::holds_alternative<ConfigMap*>(popped));

    auto v = *std::get<ConfigMap*>(popped);

    ASSERT_TRUE(v.has_property("n1_prop"));
    pop_res = v.pop_property("n1_prop");
    ASSERT_TRUE(pop_res.is_ok());
    popped = pop_res.get_ok();
    ASSERT_TRUE(std::holds_alternative<SexpCppValue>(popped));
    ASSERT_TRUE(std::holds_alternative<bool>(std::get<SexpCppValue>(popped)));
    ASSERT_TRUE(std::get<bool>(std::get<SexpCppValue>(popped)));

    ASSERT_TRUE(v.has_property("nested_2"));
    pop_res = v.pop_property("nested_2");
    ASSERT_TRUE(pop_res.is_ok());
    popped = pop_res.get_ok();
    ASSERT_TRUE(std::holds_alternative<ConfigMap*>(popped));

    auto v2{*std::get<ConfigMap*>(popped)};

    ASSERT_TRUE(v2.has_property("n2_prop"));
    pop_res = v2.pop_property("n2_prop");
    ASSERT_TRUE(pop_res.is_ok());
    popped = pop_res.get_ok();
    ASSERT_TRUE(std::holds_alternative<SexpCppValue>(popped));
    ASSERT_TRUE(std::holds_alternative<int>(std::get<SexpCppValue>(popped)));
    ASSERT_EQ(std::get<int>(std::get<SexpCppValue>(popped)), 2);

    delete nested_2;
    delete nested_1;
}

TEST(TestConfigMap, TestNestedStack)
{
    ConfigMap root_map;
    ConfigMap nested_1;
    ConfigMap nested_2;

    auto register_res = nested_1.register_property("n1_prop", ConfigValue{true});

    ASSERT_TRUE(register_res.is_ok());
    ASSERT_TRUE(nested_1.has_property("n1_prop"));

    register_res = nested_2.register_property("n2_prop", ConfigValue{2});
    ASSERT_TRUE(register_res.is_ok());
    ASSERT_TRUE(nested_2.has_property("n2_prop"));

    register_res = root_map.register_property("root_prop", ConfigValue{3.0});
    ASSERT_TRUE(root_map.has_property("root_prop"));

    register_res = nested_1.register_property("nested_2", ConfigValue{&nested_2});
    ASSERT_TRUE(register_res.is_ok());

    register_res = root_map.register_property("nested_1", &nested_1);
    ASSERT_TRUE(register_res.is_ok());

    ASSERT_TRUE(root_map.has_property("nested_1"));
    ASSERT_TRUE(root_map.has_property("root_prop"));

    auto pop_res = root_map.pop_property("root_prop");

    ASSERT_TRUE(pop_res.is_ok());

    auto popped = pop_res.get_ok();

    ASSERT_TRUE(std::holds_alternative<SexpCppValue>(popped));
    ASSERT_TRUE(std::holds_alternative<double>(std::get<SexpCppValue>(popped)));
    ASSERT_EQ(std::get<double>(std::get<SexpCppValue>(popped)), 3.0);

    pop_res = root_map.pop_property("nested_1");
    ASSERT_TRUE(pop_res.is_ok());
    popped = pop_res.get_ok();
    ASSERT_TRUE(std::holds_alternative<ConfigMap*>(popped));

    auto v = *std::get<ConfigMap*>(popped);

    ASSERT_TRUE(v.has_property("n1_prop"));
    pop_res = v.pop_property("n1_prop");
    ASSERT_TRUE(pop_res.is_ok());
    popped = pop_res.get_ok();
    ASSERT_TRUE(std::holds_alternative<SexpCppValue>(popped));
    ASSERT_TRUE(std::holds_alternative<bool>(std::get<SexpCppValue>(popped)));
    ASSERT_TRUE(std::get<bool>(std::get<SexpCppValue>(popped)));

    ASSERT_TRUE(v.has_property("nested_2"));
    pop_res = v.pop_property("nested_2");
    ASSERT_TRUE(pop_res.is_ok());
    popped = pop_res.get_ok();
    ASSERT_TRUE(std::holds_alternative<ConfigMap*>(popped));

    auto v2{*std::get<ConfigMap*>(popped)};

    ASSERT_TRUE(v2.has_property("n2_prop"));
    pop_res = v2.pop_property("n2_prop");
    ASSERT_TRUE(pop_res.is_ok());
    popped = pop_res.get_ok();
    ASSERT_TRUE(std::holds_alternative<SexpCppValue>(popped));
    ASSERT_TRUE(std::holds_alternative<int>(std::get<SexpCppValue>(popped)));
    ASSERT_EQ(std::get<int>(std::get<SexpCppValue>(popped)), 2);
}

TEST(TestConfigNest, TestConstructors)
{
    ConfigNest anonymous;

    ASSERT_STREQ(anonymous.get_name().c_str(), "");

    ConfigMap root_map;
    ConfigNest manager{"TestManager", root_map};

    ASSERT_STREQ(manager.get_name().c_str(), "TestManager");
}

TEST(TestConfigManager, TestLoadConfig)
{
    set_level(LogLevel::Info);

    /*
     * ((test-cfg . ((bool-prop . false)
     *               (int-prop . 42)
     *               (float-prop . 3.1415)
     *               (string-prop . "customized property")
     *               (symbol-prop . Odyssey)
     *              ))
     *  )
     */

    ConfigMap default_map;
    auto register_res = default_map.register_property("bool-prop", false);

    ASSERT_TRUE(register_res.is_ok());
    register_res = default_map.register_property("int-prop", 1);
    ASSERT_TRUE(register_res.is_ok());
    register_res = default_map.register_property("float-prop", 17.0);
    ASSERT_TRUE(register_res.is_ok());
    register_res = default_map.register_property("string-prop", std::string{"customized property"});
    ASSERT_TRUE(register_res.is_ok());
    register_res = default_map.register_property("symbol-prop", scheme::Symbol{"Iliad"});
    ASSERT_TRUE(register_res.is_ok());
    register_res = default_map.register_property("unmodified-prop", std::string{"unmodified property"});
    ASSERT_TRUE(register_res.is_ok());


    ConfigNest root("test-cfg", default_map);
    ConfigManager manager(root);

    auto map_res = manager.load_config_from_file("data/tests/test_config.scm");

    if (map_res.is_err())
    {
        fmt::print("{}\n", map_res.get_err().format());
    }
    ASSERT_TRUE(map_res.is_ok());
    auto new_config = map_res.get_ok();

    ASSERT_TRUE(new_config.has_property("int-prop"));
    ASSERT_TRUE(new_config.has_property("float-prop"));
    ASSERT_TRUE(new_config.has_property("string-prop"));
    ASSERT_TRUE(new_config.has_property("float-prop"));
}

}
