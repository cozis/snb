CC=gcc
CFLAGS=-Wall -Wextra -g
# CFLAGS=-Wall -Wextra -DNDEBUG -O2

.PHONY: all report clean

all: example

example: example.c config.c config.h
	$(CC) example.c config.c -o $@ $(CFLAGS) -Wpedantic

fzz: fuzz.c config.c config.h
	clang fuzz.c config.c -o $@ -g -fsanitize=fuzzer,address,undefined -O1

clean:
	rm -rf example fzz log.txt
