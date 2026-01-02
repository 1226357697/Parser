
set(
    PARSER_SRC_LIST
    Parser.h
    Parser.cpp
    main.cpp
)

add_executable(Parser ${PARSER_SRC_LIST})

target_link_libraries(Parser PRIVATE capstone)

set_target_properties(Parser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)