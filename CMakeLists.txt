cmake_minimum_required(VERSION 3.10)
project(alien)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

option(ALIEN_BUILD_TESTS "Build alien tests" OFF)
option(BUILD_SHARED_LIBS "Build shared libs" OFF)
option(ALIEN_TESTS_GTEST_USE_SHARED_CRT "Use shared CRT for testing alien with GoogleTest" ON)
option(ALIEN_MSVC_USE_DYNAMIC_RUNTIME "Use dynamic runtime library with MSVC" ON)

include(cmake/get_cpm.cmake)
include(cmake/install.cmake)
include(cmake/transfer_files.cmake)
include(build_lib.cmake)

target_compile_definitions(cxxopts INTERFACE CXXOPTS_NO_RTTI)

if(MSVC)
    add_compile_options(/utf-8)

    if(ALIEN_MSVC_USE_DYNAMIC_RUNTIME)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC Runtime Library" FORCE)
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC Runtime Library" FORCE)
    endif()
endif()

add_executable(alien bin/alien.cpp)
target_link_libraries(alien cxxopts)
target_link_libraries(alien alien_lib)

if (MINGW)
    target_compile_options(alien PUBLIC "-Wa,-mbig-obj")
    target_compile_options(alien_lib PUBLIC "-Wa, -mbig-obj")
endif()

if (ALIEN_BUILD_TESTS)
    if(ALIEN_TESTS_GTEST_USE_SHARED_CRT)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    else()
        set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
    endif()

    include(cmake/install_gtest.cmake)

    include(CTest)
    add_subdirectory(test)

    if (MINGW)
        target_compile_options(alien_tests PUBLIC "-Wa,-mbig-obj")
    endif()
endif()