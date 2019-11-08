find_library(CRITERION_LIB criterion)

SET(listVar "")
FOREACH(f IN LISTS SRC_FILES)
    LIST(APPEND listVar "../${f}")
ENDFOREACH(f)
SET(SRC_FOR_TESTS "${listVar}")

add_executable(test_string data_structures/test_string.c
    ${SRC_FOR_TESTS}
)
target_link_libraries(test_string PRIVATE
    ${CRITERION_LIB}
    common_options
)