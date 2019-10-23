# Makefile 42sh #
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -Werror
# CPPFLAGS += -D_GNU_SOURCE
.PHONY: all check debug clean

#VPATH = src
BIN = 42sh
TEST_BIN = 42sh_test
MAIN_OBJ = src/$(BIN).o

# list of all objects needed in src
OBJS = src/memory/memory.o\
       src/data_structures/data_string.o\
       #src/data_structures/array_list.o\
       src/input_output/read.o\
       src/input_output/write.o\
       src/commands/echo.o

# list of tests files
TESTS ?= tests/data_structures/test_string.c\
         #tests/input_output/test_write.c

all: $(BIN)

check: CPPFLAGS += -DDEBUG -g
check: CFLAGS += -fsanitize=address
check: clean $(OBJS)
	@echo "------------------------- 42sh tests -------------------------"
	@for test in $(TESTS); do\
	    echo "$$test";\
	    $(CC) $(CFLAGS) -o $(TEST_BIN) $$test $(OBJS) -lcriterion;\
	    ./$(TEST_BIN);\
	done
	@echo "--------------------------------------------------------------"

debug: CPPFLAGS += -DDEBUG -g
debug: clean $(BIN)

$(BIN): $(MAIN_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(BIN) $(OBJS) $(MAIN_OBJ) $(TEST_BIN)
