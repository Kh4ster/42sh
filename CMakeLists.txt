cmake_minimum_required(VERSION 3.8)
find_program(CMAKE_C_COMPILER
    NAMES $ENV{CC} gcc
    PATHS ENV PATH NO_DEFAULT_PATH
)
project(42sh VERSION 1.0.0 LANGUAGES C)

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
    src/input_output/read.c
    src/input_output/write.c
    src/execution_handling/command_container.c
    #src/execution_handling/redirector.c
    src/data_structures/array_list.c
    src/parameters_handling/parameters_handler.c
    src/lexer/token_lexer.c
)

# ---- List of test files (unit tests) ----
set(TESTS_FILES
    tests/data_structures/test_string.c
    tests/input_output/test_read.c
    tests/input_output/test_write.c
    #tests/builtins/test_redirector.c
    tests/parameters_handling/test_parameters_handler.c
    tests/data_structures/test_queue.c
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

# Create an interface containing options needed by compiler and linker
add_library(common_options INTERFACE)
target_compile_options(common_options INTERFACE
    #[[Basic options]]
    "-D_GNU_SOURCE"
    "-Wall"
    "-Wextra"
    "-pedantic"
    #[[Active -Werror on build_release and sanitize on build_debug]]
    $<IF:$<CONFIG:Release>,-Werror,-fsanitize=address>
)
target_link_libraries(common_options INTERFACE
    $<$<NOT:$<CONFIG:Release>>:-fsanitize=address>
    "-lreadline"
)


# add "../" prefix to src files to use them in sub-directories
set(listVar "")
foreach(f IN LISTS SRC_FILES)
    list(APPEND listVar "../${f}")
endforeach(f)
set(SRC_FILES_FROM_SUBDIR "${listVar}")
# Create a library containing all src files
add_library(SRC_FILES_OBJ ${SRC_FILES_FROM_SUBDIR})
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
