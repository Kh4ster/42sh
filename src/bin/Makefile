CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -Werror
CPPFLAGS += -D_GNU_SOURCE

all:
	gcc echo.c ../input_output/read.c ../input_output/write.c ../data_structures/data_string.c ../memory/memory.c -g -o echo
	gcc cat.c ../input_output/read.c ../input_output/write.c ../data_structures/data_string.c ../memory/memory.c -o cat
	gcc -g ../execution_handling/redirector.c 