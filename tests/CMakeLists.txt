find_library(CRITERION_LIB criterion)

set(TESTS_FILES
    data_structures/test_string.c
    input_output/test_read.c
    input_output/test_write.c
)

SET(listVar "")
FOREACH(f IN LISTS SRC_FILES)
    LIST(APPEND listVar "../${f}")
ENDFOREACH(f)
SET(SRC_FOR_TESTS "${listVar}")

enable_testing()
foreach(f IN LISTS TESTS_FILES)
    string(REGEX REPLACE "\\.[^.]*$" "" F_WITHOUT_EXT ${f})
    string(REGEX REPLACE "[^/]*/" "" TEST_NAME ${F_WITHOUT_EXT})
    add_executable(${TEST_NAME} ${f}
        ${SRC_FOR_TESTS}
    )
    target_link_libraries(${TEST_NAME} PRIVATE
        ${CRITERION_LIB}
        common_options
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach(f)