cmake_minimum_required(VERSION 3.2)
project(alien)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

include(cmake/get_cpm.cmake)
include(cmake/install.cmake)

add_subdirectory(include)

configure_file(include/templates/lexer.template.txt ${CMAKE_CURRENT_BINARY_DIR}/templates/lexer.template.txt COPYONLY)
configure_file(include/templates/token.template.txt ${CMAKE_CURRENT_BINARY_DIR}/templates/token.template.txt COPYONLY)
configure_file(include/templates/parser.template.txt ${CMAKE_CURRENT_BINARY_DIR}/templates/parser.template.txt COPYONLY)

add_executable(alien bin/front.cpp)
target_link_libraries(alien cxxopts)
target_link_libraries(alien alien_lib)