message(STATUS "Alien: Building Tests")
include(GoogleTest)

enable_testing()

file(GLOB_RECURSE TEST_SOURCES *.cpp)

message(STATUS "Alien: Test files - ${TEST_SOURCES}")

add_executable(alien_tests ${TEST_SOURCES})

target_link_libraries(alien_tests PRIVATE GTest::gtest GTest::gtest_main)
target_link_libraries(alien_tests PRIVATE alien_lib)

gtest_discover_tests(alien_tests TEST_PREFIX alien:)