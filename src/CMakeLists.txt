# Create an executable named 42sh from file 42sh.c
add_executable(42sh 42sh.c
)

# Set Properties of the 42sh executable
set_target_properties(42sh PROPERTIES
    C_STANDARD 99 # set the target C standard to use
    C_EXTENSIONS OFF # avoid using std=gnu99 and force std=c99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} # where 42sh should be created
)

# Set common options of compilations (-Wall...) and link all src_files needed
target_link_libraries(42sh PRIVATE
    common_options
    SRC_FILES_OBJ
)