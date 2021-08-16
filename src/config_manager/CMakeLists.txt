add_samos_minimal_target(
    ConfigManager
    SOURCES src/config_manager.cpp
    TEST_SOURCES test/test_config_manager.cpp
    EXTRA_LIBS chibi-scheme
    SAMOS_DEPS Result Logger Scheme)
