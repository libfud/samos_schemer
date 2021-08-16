add_samos_target(EdLine
    src/ed_line.cpp
    test/test_ed_line.cpp
    EXTRA_INCS replxx
    EXTRA_LIBS replxx
)
target_link_libraries(EdLine PUBLIC replxx)
