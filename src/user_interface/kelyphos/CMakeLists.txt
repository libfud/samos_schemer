add_samos_target(
    Kelyphos
    src/kelyphos.cpp
    test/test_kelyphos.cpp
    EXTRA_LIBS chibi-scheme
    EXTRA_INCS chibi-scheme
    SAMOS_DEPS EdLine Scheme ConfigManager
    )
