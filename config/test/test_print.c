#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../config.h"
#include "test.h"
#include "test_print.h"

static FILE *stream;

void
log_result(int line, bool failed)
{
    if (failed)
        fprintf(stream, "SUCCESS CASE - " RED "FAILED " RESET "(%s:%d)\n", __FILE__,
                line);
    else
        fprintf(stream, "SUCCESS CASE - " GREEN "PASSED " RESET "(%s:%d)\n",
                __FILE__, line);
}

bool
assert_eq_prints(const char *expected, CfgError *err)
{
    char buffer[256];
    FILE *tmp = fmemopen(buffer, sizeof(buffer), "w");
    if (tmp == NULL) {
        fprintf(stream, "FATAL: fmemopen() failed");
        return false;
    }

    cfg_fprint_error(tmp, err);
    fclose(tmp);

    return !strncmp(buffer, expected, strlen(expected));
}

void
run_print_err_test_1(Scoreboard *scoreboard)
{
    Cfg cfg;
    CfgEntry entries[TEST_CAPACITY];

    cfg_init(&cfg, entries, TEST_CAPACITY);

    CfgError err;
    cfg_load("", &cfg, &err);

    scoreboard->total++;

    if (!assert_eq_prints("Error: invalid filename\n", &err)) {
        cfg_fprint_error(stream, &err);
        log_result(__LINE__, true);
        scoreboard->failed++;
    } else {
        log_result(__LINE__, false);
        scoreboard->passed++;
    }
}

void
run_print_err_test_2(Scoreboard *scoreboard)
{
    Cfg cfg;
    CfgEntry entries[TEST_CAPACITY];

    cfg_init(&cfg, entries, TEST_CAPACITY);

    CfgError err;
    const char *src = "a:true\nb:";
    cfg_parse(src, strlen(src), &cfg, &err);

    scoreboard->total++;

    if (!assert_eq_prints("Error at 2:3 :: missing value\n", &err)) {
        cfg_fprint_error(stream, &err);
        log_result(__LINE__, true);
        scoreboard->failed++;
    } else {
        log_result(__LINE__, false);
        scoreboard->passed++;
    }
}

static TestResult
run_print_test_on_source(const char *src)
{
    Cfg cfg;
    CfgError error;
    CfgEntry entries[1024];

    cfg_init(&cfg, entries, COUNT_OF(entries));
    cfg_parse(src, &cfg, &err);

    char buffer[256];
    FILE *tmp = fmemopen(buffer, sizeof(buffer), "w");
    ASSERT_CONTEXT(tmp != NULL); // If this triggers, "tmp" will leak

    cfg_fprint(tmp, cfg);

    fclose(tmp);

    ASSERT( !strncmp(buffer, expected, strlen(expected)) );

    return OK;
}

static TestResult
run_print_test(void)
{
    static const char src[] = 
        "font: \"JetBrainsMono Nerd Font\"\n"
        "font.size: 14\n"
        "zoom: 1.500000\n"
        "line_numbers: true\n"
        "ruler: false\n"
        "bg.color: rgba(255, 255, 255, 255)";
    return run_print_test_on_source(src);
}

void
run_print_tests(Scoreboard *scoreboard, FILE *stream_)
{
    TestResult result = run_print_test(scoreboard);
    run_print_err_test_1(scoreboard);
    run_print_err_test_2(scoreboard);
}
