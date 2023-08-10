CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -g
# CFLAGS=-Wall -Wextra -Wpedantic -O2

.PHONY: all clean

all: example

example: example.c config.c config.h
	$(CC) example.c config.c -o $@ $(CFLAGS)

test: test.c config.c config.h
	$(CC) test.c config.c -o $@ $(CFLAGS)

fuzz: fuzz.c config.c config.h
	clang -O1 -g -fsanitize=fuzzer,address,undefined fuzz.c config.c -o $@

clean:
	rm example test fuzz
