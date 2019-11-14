# Find Criterion library
find_library(CRITERION_LIB criterion)

# Set good path for tests files
set(listVar "")
foreach(f IN LISTS TESTS_FILES)
    list(APPEND listVar "../${f}")
endforeach(f)
set(TESTS_FILES "${listVar}")

if (CRITERION_LIB)
# For each test file create an executable of test. Launched using ctest
foreach(f IN LISTS TESTS_FILES)
    # Find the test_name by deleting the extension and the parent_dir
    string(REGEX REPLACE "\\.[^.]*$" "" F_WITHOUT_EXT ${f})
    string(REGEX REPLACE "[^/]*/" "" TEST_NAME ${F_WITHOUT_EXT})

    # Create an executable linked to all src and compilation options needed
    add_executable(${TEST_NAME} ${f})
    target_link_libraries(${TEST_NAME} PRIVATE
        ${CRITERION_LIB}
        common_options
        SRC_FILES_OBJ
    )

    # Add the executable created to the test list of ctest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    # Set timeout of the test to 3 seconds
    set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT 1)
endforeach(f)
else ()
    message(WARNING "Criterion was not found... You will not be able to launch tests")
endif ()