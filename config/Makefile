CC=gcc
CFLAGS=-Wall -Wextra -g
# CFLAGS=-Wall -Wextra -DNDEBUG -O2
TST_SRC=test/test.c test/test_parse.c test/test_load.c test/test_get.c test/test_print.c

.PHONY: all report clean

all: example

example: example.c config.c config.h
	$(CC) example.c config.c -o $@ $(CFLAGS) -Wpedantic

fzz: fuzz/fuzz.c config.c config.h
	clang -O1 -g -fsanitize=fuzzer,address,undefined fuzz/fuzz.c config.c -o $@

tst: $(TST_SRC) config.c config.h
	$(CC) $(TST_SRC) config.c -o $@ $(CFLAGS)

test-cov: $(TST_SRC) config.c config.h
	$(CC) $(TST_SRC) config.c -o $@ $(CFLAGS) -fprofile-arcs -ftest-coverage -DNDEBUG

report: clean test-cov
	./test-cov
	lcov --rc lcov_branch_coverage=1 --capture --directory . --output-file coverage.info
	@ mkdir -p report
	genhtml coverage.info --output-directory report --branch-coverage

clean:
	rm -rf example fzz tst test-cov \
	       test-cov-*.gcda test-cov-*.gcno coverage.info \
		   log.txt report/

	find ./fuzz/corpus -type f ! -name '*.cfg' -delete

