CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -g
# CFLAGS=-Wall -Wextra -Wpedantic -O2

.PHONY: all clean

all: example

example: example.c config.c config.h
	$(CC) example.c config.c -o $@ $(CFLAGS)

fuzz: fuzz.c config.c
	clang -O1 -g -fsanitize=fuzzer,address,undefined $^ -o $@

clean:
	rm example fuzz
