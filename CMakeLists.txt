cmake_minimum_required(VERSION 3.18)
project(alien)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

include(cmake/get_cpm.cmake)
include(cmake/install.cmake)

add_subdirectory(include)

add_executable(alien-front bin/front.cpp)
target_link_libraries(alien-front cxxopts)
target_link_libraries(alien-front alien)

if (MSVC)
    target_compile_options(alien-front PUBLIC "${CMAKE_CXX_FLAGS} /bigobj")
else()
    target_compile_options(alien-front PUBLIC "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
endif()