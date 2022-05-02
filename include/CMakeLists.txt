add_library(${PROJECT_NAME}_lib INTERFACE)

target_include_directories(${PROJECT_NAME}_lib INTERFACE .)
target_include_directories(${PROJECT_NAME}_lib INTERFACE ../deps)

target_link_libraries(${PROJECT_NAME}_lib INTERFACE utf8proc)

