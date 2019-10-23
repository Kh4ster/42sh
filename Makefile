# Makefile 42sh #
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -Werror
# CPPFLAGS += -D_GNU_SOURCE
.PHONY: all check debug clean

#VPATH = src
BIN = 42sh
MAIN_OBJ = src/$(BIN).o

# list of all objects needed in src
OBJS = src/memory/memory.o\
       src/data_structures/data_string.o

# list of tests files
TESTS ?= tests/data_structures/test_string.c

all: $(BIN)

check: CPPFLAGS += -DDEBUG -g
check: CFLAGS += -fsanitize=address
check: clean $(OBJS)
	echo "------------------------- 42sh tests -------------------------"
	for test in $(TESTS); do\
	    echo "$$test";\
	    $(CC) $(CFLAGS) -o 42sh_test $$test $(OBJS) -lcriterion;\
	    ./42sh_test;\
	done
	echo "--------------------------------------------------------------"

debug: CPPFLAGS += -DDEBUG -g
debug: clean $(BIN)

$(BIN): $(MAIN_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(BIN) $(OBJS) $(MAIN_OBJ)
