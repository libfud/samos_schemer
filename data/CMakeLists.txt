configure_file(example.scm ${CMAKE_CURRENT_BINARY_DIR}/example.scm COPYONLY)
file(GLOB SCM_TEST_FILES "tests/*.scm")
file(COPY ${SCM_TEST_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/tests/)
