cmake_minimum_required(VERSION 3.10)
project(alien)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

option(ALIEN_BUILD_TESTS "Build alien tests" OFF)

include(cmake/get_cpm.cmake)
include(cmake/install.cmake)
include(cmake/transfer_files.cmake)
include(build_lib.cmake)

target_compile_definitions(cxxopts INTERFACE CXXOPTS_NO_RTTI)

add_executable(alien bin/alien.cpp)
target_link_libraries(alien cxxopts)
target_link_libraries(alien alien_lib)

if (MINGW)
    target_compile_options(alien PUBLIC "-Wa,-mbig-obj")
    target_compile_options(alien_lib PUBLIC "-Wa, -mbig-obj")
endif()

if (ALIEN_BUILD_TESTS)
    include(CTest)
    add_subdirectory(test)

    if (MINGW)
        target_compile_options(alien_tests PUBLIC "-Wa,-mbig-obj")
    endif()
endif()