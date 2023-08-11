CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -g
# CFLAGS=-Wall -Wextra -Wpedantic -DNDEBUG -O2

.PHONY: all clean

all: example

example: example.c config.c config.h
	$(CC) example.c config.c -o $@ $(CFLAGS)

test: test.c config.c config.h
	$(CC) test.c config.c -o $@ $(CFLAGS)

fuzz: fuzz.c config.c config.h
	clang -O1 -g -fsanitize=fuzzer,address,undefined fuzz.c config.c -o $@

test-cov: test.c config.c config.h
	$(CC) test.c config.c -o $@ $(CFLAGS) -fprofile-arcs -ftest-coverage -DNDEBUG

report: clean test-cov
	./test-cov
	lcov --rc lcov_branch_coverage=1 --capture --directory . --output-file coverage.info
	@ mkdir -p report
	genhtml coverage.info --output-directory report --branch-coverage

clean:
	rm -f example test fuzz log.txt test-cov-*.gcda test-cov-*.gcno
