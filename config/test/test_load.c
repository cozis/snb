#include <stdio.h>
#include <string.h>

#include "../config.h"
#include "test.h"
#include "test_load.h"

typedef struct {
    enum { TC_SUCC, TC_ERR } type;
    int line;
    const char *filename;
    // TC_ERR
    const char *err;  // Expected error message
} TestCase;

static const TestCase test_cases[] = {
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .filename = "sample.cfg",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .filename = "sample.txt",
        .err = "invalid file extension",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .filename = "",
        .err = "invalid filename",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .filename = "sample2.cfg",
        .err = "failed to open the file",
    },
};

static FILE *stream;

static void
log_result(TestCase tc, bool failed)
{
    char *prefix = tc.type ? "ERROR   CASE" : "SUCCESS CASE";

    if (failed) {
        fprintf(stream, "%s - " RED "FAILED " RESET "(%s:%d)\n", prefix, __FILE__,
                tc.line);
    } else {
        fprintf(stream, "%s - " GREEN "PASSED " RESET "(%s:%d)\n", prefix, __FILE__,
                tc.line);
    }
}

static void
run_test_case(TestCase tc, Scoreboard *scoreboard)
{
    Cfg cfg;
    CfgEntry entries[TEST_CAPACITY];

    cfg_init(&cfg, entries, TEST_CAPACITY);

    CfgError err;
    int res = cfg_load(tc.filename, &cfg, &err);

    switch (tc.type) {
    case TC_SUCC:
        if (res != 0) {
            // SUCCESS CASE - FAILED
            cfg_fprint_error(stream, &err);
            log_result(tc, true);
            scoreboard->failed++;
        } else {
            // SUCCESS CASE - PASSED
            log_result(tc, false);
            scoreboard->passed++;
        }
        break;

    case TC_ERR:
        if (res == 0) {
            // ERROR CASE - FAILED (successful parsing)
            fprintf(stream, "Error case was parsed successfully\n");
            log_result(tc, true);
            scoreboard->failed++;
        } else {
            if (strcmp(tc.err, err.msg) != 0) {
                // ERROR CASE - FAILED (error message mismatch)
                cfg_fprint_error(stderr, &err);
                log_result(tc, true);
                scoreboard->failed++;
            } else {
                // ERROR CASE - PASSED
                log_result(tc, false);
                scoreboard->passed++;
            }
        }
        break;
    }
}

void
run_load_tests(Scoreboard *scoreboard, FILE *stream_)
{
    stream = stream_;

    int total = sizeof(test_cases) / sizeof(test_cases[0]);
    scoreboard->total += total;

    for (int i = 0; i < total; i++) {
        run_test_case(test_cases[i], scoreboard);
    }
}
