find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_samos_minimal_target(
    Logger
    SOURCES src/logger.cpp
    TEST_SOURCES test/test_logger.cpp
    EXTRA_LIBS fmt spdlog::spdlog)
