cmake_minimum_required(VERSION 3.19)
project(alien)

set(CMAKE_CXX_STANDARD 17)

include(cmake/get_cpm.cmake)
include(cmake/install.cmake)

add_subdirectory(include)

add_executable(alien-front bin/front.cpp)
target_link_libraries(alien-front cxxopts)
target_link_libraries(alien-front alien)
#target_link_libraries(alien-front -static-libgcc -static-libstdc++)

add_executable(cl bin/cl.cpp)
target_link_libraries(cl alien)

add_executable(lex bin/lexer.alien.cpp)