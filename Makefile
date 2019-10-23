# Makefile 42sh #
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -Werror
# CPPFLAGS += -D_GNU_SOURCE

VPATH = src
BIN = 42sh
MAIN_OBJ = $(BIN).o

# list of all objects needed in src
OBJS = memory.o

# list list of tests objs
TESTS_OBJS = #tests/*.o

all: $(BIN)

check: CPPFLAGS += -DDEBUG -g
check: CFLAGS += -fsanitize=address
check: clean $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(TESTS_OBJS)

debug: CPPFLAGS += -DDEBUG -g
debug: clean $(BIN)

$(BIN): $(MAIN_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(BIN) $(OBJS) $(MAIN_OBJ)
