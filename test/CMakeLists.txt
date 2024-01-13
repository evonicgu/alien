message(STATUS "Alien: Building Tests")

set(gtest_force_shared_crt ON)

include(GoogleTest)

enable_testing()

file(GLOB_RECURSE TEST_SOURCES *.cpp)

message(STATUS "Alien: Test files - ${TEST_SOURCES}")

add_executable(alien_tests ${TEST_SOURCES})

set(gtest_force_shared_crt ON)
target_link_libraries(alien_tests GTest::gtest GTest::gtest_main)
target_link_libraries(alien_tests alien_lib)

gtest_discover_tests(alien_tests)

add_test(all_gtest alien_tests)