if(NOT DEFINED ENV{NO_CLANG_CHECK})
    set(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif()

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -Wunused-variable)

add_library(samos_lib INTERFACE)
add_executable(samos samos.cpp)
add_executable(kelyphos kelyphos.cpp)
add_executable(metaforeas metaforeas.cpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Provides Result
add_subdirectory(result)
# Provides Logging
add_subdirectory(log)
# Provides Scheme
add_subdirectory(scheme)
# Provides ConfigManager
add_subdirectory(config_manager)
# Provides Kelyphos, EdLine, OptionParser
add_subdirectory(user_interface)

add_library(lib_samos SHARED lib_samos.cpp)

target_link_libraries(lib_samos PUBLIC samos_lib fmt Eigen3::Eigen)
target_link_libraries(samos PUBLIC lib_samos)
target_link_libraries(kelyphos PUBLIC lib_samos)
target_link_libraries(metaforeas PUBLIC lib_samos)
install(TARGETS samos DESTINATION bin)
