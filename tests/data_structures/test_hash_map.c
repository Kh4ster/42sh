#include <criterion/criterion.h>

#include "../../src/data_structures/hash_map.h"
#include "../../src/execution_handling/command_container.h"

int main(void)
{
    struct hash_map map;
    hash_init(&map, 1);

    struct command_container *echo_container = command_init(1, "echo", "ok");
    hash_insert(&map, "echo", build_instruction(TOKEN_COMMAND, echo_container));

    struct command_container *cat_container = command_init(0, "cat");
    hash_insert(&map, "cat", build_instruction(TOKEN_COMMAND, cat_container));

    struct instruction *echo_instruction = hash_find(&map, "echo");
    cr_assert_not_null(echo_instruction);
    cr_assert_eq(TOKEN_COMMAND, echo_instruction->type);
    cr_assert_eq(echo_container, echo_instruction->data);

    struct instruction *cat_instruction = hash_find(&map, "cat");
    cr_assert_not_null(cat_instruction);
    cr_assert_eq(TOKEN_COMMAND, cat_instruction->type);
    cr_assert_eq(cat_container, cat_instruction->data);

    hash_free(&map);
}