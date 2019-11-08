add_executable(42sh 42sh.c
    ${SRC_FILES_FOR_TESTS}
)
set_target_properties(42sh PROPERTIES
    C_STANDARD 99
    C_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)
target_link_libraries(42sh PRIVATE
    common_options
)