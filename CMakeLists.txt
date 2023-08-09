cmake_minimum_required(VERSION 3.2)
project(alien)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

include(cmake/get_cpm.cmake)
include(cmake/install.cmake)

configure_file(resources/templates/cpp/sources/lexer.template.txt ${CMAKE_CURRENT_BINARY_DIR}/resources/templates/cpp/sources/lexer.template.txt COPYONLY)
configure_file(resources/templates/cpp/sources/token.template.txt ${CMAKE_CURRENT_BINARY_DIR}/resources/templates/cpp/sources/token.template.txt COPYONLY)
configure_file(resources/templates/cpp/sources/parser.template.txt ${CMAKE_CURRENT_BINARY_DIR}/resources/templates/cpp/sources/parser.template.txt COPYONLY)
configure_file(resources/templates/cpp/headers/lexer.template.txt ${CMAKE_CURRENT_BINARY_DIR}/resources/templates/cpp/headers/lexer.template.txt COPYONLY)
configure_file(resources/templates/cpp/headers/parser.template.txt ${CMAKE_CURRENT_BINARY_DIR}/resources/templates/cpp/headers/parser.template.txt COPYONLY)

target_compile_definitions(cxxopts INTERFACE CXXOPTS_NO_RTTI)


file(GLOB_RECURSE SOURCES src/*.cpp)

message(STATUS ${SOURCES})

add_library(alien_lib ${SOURCES})

target_include_directories(alien_lib PUBLIC include)

target_include_directories(alien_lib PUBLIC deps)
target_link_libraries(alien_lib PUBLIC utf8proc)
target_link_libraries(alien_lib PUBLIC Boost::container_hash)


add_executable(alien bin/alien.cpp)
target_link_libraries(alien cxxopts)
target_link_libraries(alien alien_lib)

target_compile_options(alien PUBLIC -fno-omit-frame-pointer)