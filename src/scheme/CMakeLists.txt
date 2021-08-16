add_samos_minimal_target(
    Scheme
    SOURCES src/scheme.cpp
    TEST_SOURCES test/test_scheme.cpp
    SAMOS_DEPS Result Logger
    EXTRA_LIBS chibi-scheme
    )
