file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(alien_lib ${SOURCES})

target_include_directories(alien_lib PUBLIC include)

target_include_directories(alien_lib PUBLIC deps)
target_link_libraries(alien_lib PUBLIC utf8proc)
target_link_libraries(alien_lib PUBLIC Boost::container_hash)