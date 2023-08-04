CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -g
# CFLAGS=-Wall -Wextra -Wpedantic -O3

.PHONY: all clean

all: config

config: main.c config.c
	$(CC) $^ -o $@ $(CFLAGS)

fuzz: fuzz.c config.c
	clang -O1 -g -fsanitize=fuzzer,address $^ -o $@

clean:
	rm config fuzz
