cmake_minimum_required(VERSION 3.8)
find_program(CMAKE_C_COMPILER
    NAMES $ENV{CC} gcc
    PATHS ENV PATH NO_DEFAULT_PATH
)
project(42sh VERSION 1.2.1 LANGUAGES C)

#[[
----- USAGE -----
1. Create and go to a directory of build (preference : debug or release) :
    - `mkdir build_debug` / `mkdir build_release`
    - `cd build_debug` / `cd build_release`

2. Generate cmake files in build directory with the good options :
    - `cmake .. [OPTIONS]`
        - Set Compiler  : -DCMAKE_C_COMPILER=/usr/bin/gcc
        - Debug build   : -DCMAKE_BUILD_TYPE=Debug
        - Release build : -DCMAKE_BUILD_TYPE=Release

3. Use make as usual in the wanted build_directory:
    - `make`
    - `make all`
    - `make clean`
    - `make check`
    - `make -B`
    - verbose option : `VERBOSE=1 make`

4. verbose tests
    - ctest --verbose
]]

# ---- List of source files ----
set(SRC_FILES
    src/memory/memory.c
    src/data_structures/data_string.c
    src/data_structures/queue.c
    src/data_structures/hash_map.c
    src/input_output/read.c
    src/execution_handling/command_container.c
    src/data_structures/array_list.c
    src/parameters_handling/parameters_handler.c
    src/lexer/token_lexer.c
    src/execution_handling/command_execution.c
    src/parser/parser.c
    src/parser/ast/ast.c
    src/parser/ast/destroy_tree.c
    src/input_output/get_next_line.c
    src/error/error.c
    src/parser/ast/ast_print.c
    src/redirections_handling/redirect.c
    src/builtins/cd.c
    src/builtins/shopt.c
    src/builtins/echo.c
    src/builtins/history.c
    src/builtins/source.c
    src/builtins/break.c
    src/builtins/exit.c
    src/builtins/export.c
    src/builtins/alias/alias.c
    src/builtins/alias/unalias.c
    src/path_expention/path_exepension.c
    src/arithmetic_expression/parser.c
    src/arithmetic_expression/lexer.c
    src/arithmetic_expression/tree.c
    src/data_structures/stack.c
    src/special_variables/expand_special_variables.c
    src/command_substitution/command_substitution.c
    src/input_output/prompt.c
    src/command_expansion/command_expansion.c
    src/parser/ast_build.c
    src/parser/parser_utils.c
)

# ---- List of test files (unit tests) ----
set(TESTS_FILES
    tests/data_structures/test_string.c
    tests/input_output/test_read.c
    #tests/builtins/test_redirector.c
    tests/builtins/test_shopt.c
    tests/parameters_handling/test_parameters_handler.c
    #tests/commands_handling/test_exec.c
    #tests/commands_handling/test_create.c
    tests/data_structures/test_queue.c
    tests/data_structures/test_hash_map.c
    tests/lexer/test_token_lexer.c
)

# Set default BUILD_TYPE
if(NOT CMAKE_BUILD_TYPE)
    string(REGEX REPLACE ".*/build_" "" BUILD_NAME ${CMAKE_BINARY_DIR})
    set(CMAKE_BUILD_TYPE Release)
    if(${BUILD_NAME} STREQUAL "Debug" OR ${BUILD_NAME} STREQUAL "debug")
        set(CMAKE_BUILD_TYPE Debug)
    endif (${BUILD_NAME} STREQUAL "Debug" OR ${BUILD_NAME} STREQUAL "debug")
    message(STATUS "Build type : ${CMAKE_BUILD_TYPE}")
endif(NOT CMAKE_BUILD_TYPE)

# Find Readline library
find_library(READLINE_LIB readline)

# Create an interface containing options needed by compiler and linker
add_library(common_options INTERFACE)
target_compile_options(common_options INTERFACE
    #[[Basic options]]
    "-D_GNU_SOURCE"
    "-D_POSIX_C_SOURCE"
    "-Wall"
    "-Wextra"
    "-pedantic"
    "-std=c99"
    "-O0"
    #[[Active -Werror on build_release and sanitize on build_debug]]
    $<IF:$<CONFIG:Release>,-Werror,-fsanitize=address>
)
target_link_libraries(common_options INTERFACE
    $<$<NOT:$<CONFIG:Release>>:-fsanitize=address>
    ${READLINE_LIB}
    "-lm"
)


# add "../" prefix to src files if build in sub_dir (cmake ..)
if(NOT ${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(listVar "")
    foreach(f IN LISTS SRC_FILES)
        list(APPEND listVar "../${f}")
    endforeach(f)
    set(SRC_FILES "${listVar}")
endif(NOT ${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})


# Create a library containing all src files
add_library(SRC_FILES_OBJ ${SRC_FILES})
target_link_libraries(SRC_FILES_OBJ PRIVATE
    common_options
)


# Create a check.sh to launch custom target "check"
file(WRITE "${CMAKE_BINARY_DIR}/tmp/check.sh"
    "make; python3 ${CMAKE_SOURCE_DIR}/tests/42sh_tests.py ${CMAKE_SOURCE_DIR}/42sh; ctest"
)
file(
    COPY "${CMAKE_BINARY_DIR}/tmp/check.sh"
    DESTINATION "${CMAKE_BINARY_DIR}"
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
# Add custom target named check launching check.sh script
add_custom_target(check
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMAND "${CMAKE_BINARY_DIR}/check.sh"
)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
