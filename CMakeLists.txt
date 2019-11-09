cmake_minimum_required(VERSION 3.8)
project(42sh VERSION 1.0.0 LANGUAGES C)

# Set Compiler : -DCMAKE_C_COMPILER=/usr/bin/gcc
# Debug : -DCMAKE_BUILD_TYPE=Debug
# Release : -DCMAKE_BUILD_TYPE=Release

add_library(common_options INTERFACE)
target_compile_options(common_options INTERFACE
    "-D_GNU_SOURCE"
    "-Wall"
    "-Wextra"
    "-pedantic"
    $<IF:$<CONFIG:Release>,-Werror,-fsanitize=address>
)
target_link_libraries(common_options INTERFACE
    $<$<NOT:$<CONFIG:Release>>:-fsanitize=address>
)

set(SRC_FILES
    src/memory/memory.c
    src/data_structures/data_string.c
    src/input_output/read.c
    src/input_output/write.c
    src/execution_handling/command_container.c
    src/execution_handling/redirector.c
    src/data_structures/array_list.c
)

set(CHECK_FILE "${CMAKE_BINARY_DIR}/check.log")
file(WRITE "${CMAKE_BINARY_DIR}/tmp/check.sh" "make; ctest")
file(
    COPY "${CMAKE_BINARY_DIR}/tmp/check.sh"
    DESTINATION "${CMAKE_BINARY_DIR}"
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
add_custom_command(OUTPUT ${CHECK_FILE}
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMAND "${CMAKE_BINARY_DIR}/check.sh"
)
add_custom_target(check DEPENDS ${CHECK_FILE})

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)