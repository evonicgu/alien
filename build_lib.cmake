file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(alien_lib ${SOURCES})

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_output)

if(lto_supported)
    set_property(TARGET alien_lib PROPERTY CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    message(STATUS "LTO/IPO is enabled for Release configuration")
else()
    message(STATUS "LTO/IPO requested but it is not supported by the compiler: ${lto_output}")
endif()

target_include_directories(alien_lib PUBLIC include)

target_include_directories(alien_lib PUBLIC deps)
target_link_libraries(alien_lib PUBLIC utf8proc)