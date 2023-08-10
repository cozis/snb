CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -g
# CFLAGS=-Wall -Wextra -Wpedantic -O2

.PHONY: all clean

all: config

config: main.c config.c config.h
	$(CC) main.c config.c -o $@ $(CFLAGS)

fuzz: fuzz.c config.c
	clang -O1 -g -fsanitize=fuzzer,address,undefined $^ -o $@

clean:
	rm config fuzz
