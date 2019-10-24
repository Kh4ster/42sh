# Makefile 42sh #
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -Werror
CPPFLAGS += -D_GNU_SOURCE
.PHONY: all check debug clean

#VPATH = src
BIN = 42sh
MAIN_OBJ = src/$(BIN).o

# list of all objects needed in src
OBJS = src/memory/memory.o\
       src/data_structures/data_string.o\
       src/input_output/read.o\
       src/input_output/write.o\
       #src/data_structures/array_list.o\
       src/commands/echo.o

# list of tests files
TESTS ?= tests/data_structures/test_string.c\
         tests/input_output/test_read.c

all: $(BIN)

check: CPPFLAGS += -DDEBUG -g
check: CFLAGS += -fsanitize=address
check: clean $(OBJS)
	@echo "------------------------- 42sh tests -------------------------"
	@mkdir -p bin
	@for test in $(TESTS); do\
	    echo "$$test";\
	    bin_n=$$(echo "$$test" | rev | cut -d "/" -f1 | tr [/] [_] | rev\
	    | cut -d "." -f1);\
	    $(CC) $(CFLAGS) -o bin/$(BIN)_$$bin_n $$test $(OBJS) -lcriterion;\
	    ./bin/$(BIN)_$$bin_n;\
	done
	@echo "--------------------------------------------------------------"

debug: CPPFLAGS += -DDEBUG -g
debug: clean $(BIN)

$(BIN): $(MAIN_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(BIN) $(OBJS) $(MAIN_OBJ) bin
