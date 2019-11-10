CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -Werror
CPPFLAGS += -D_GNU_SOURCE

all:
	$(CC) $(CFLAGS) $(CPPFLAGS) echo.c ../input_output/read.c ../input_output/write.c ../data_structures/data_string.c ../memory/memory.c -g -o echo
	$(CC) $(CFLAGS) $(CPPFLAGS) cat.c ../input_output/read.c ../input_output/write.c ../data_structures/data_string.c ../memory/memory.c -o cat