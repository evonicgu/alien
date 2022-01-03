add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE .)
target_include_directories(${PROJECT_NAME} INTERFACE ../deps)

target_link_libraries(${PROJECT_NAME} INTERFACE utf8proc)

configure_file(templates/lexer.template.txt ${CMAKE_BINARY_DIR}/templates/lexer.template.txt COPYONLY)
configure_file(templates/parser.template.txt ${CMAKE_BINARY_DIR}/templates/parser.template.txt COPYONLY)