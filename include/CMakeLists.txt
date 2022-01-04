add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE .)
target_include_directories(${PROJECT_NAME} INTERFACE ../deps)

target_link_libraries(${PROJECT_NAME} INTERFACE utf8proc)

