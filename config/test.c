#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

// #define LOGFILE
#define TEST_MAX_ENTRIES 64

#ifdef LOGFILE
#define RED
#define GREEN
#define RESET
#else
#define RED "\e[1;31m"
#define GREEN "\e[1;32m"
#define RESET "\e[0m"
#endif

typedef struct {
    enum { TC_SUCC, TC_ERR } type;
    int line;
    const char *src;
    int max_entries;
    // TC_SUCC (Parsing)
    CfgEntry *entries;  // Expected entries
    int size;           // Expected size of [entries]
    // TC_ERR (Parsing/Loading)
    const char *err;  // Expected error message
} TestCase;

typedef struct {
    int total;
    int passed;
    int failed;
} Scoreboard;

static const TestCase parsing_test_cases[] = {
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "",
        .max_entries = TEST_MAX_ENTRIES,
        .entries = (CfgEntry[]){},
        .size = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = " ",
        .max_entries = TEST_MAX_ENTRIES,
        .entries = (CfgEntry[]){},
        .size = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "#",
        .max_entries = TEST_MAX_ENTRIES,
        .entries = (CfgEntry[]){},
        .size = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "#\n",
        .max_entries = TEST_MAX_ENTRIES,
        .entries = (CfgEntry[]){},
        .size = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "x",
        .max_entries = 0,
        .entries = (CfgEntry[]){},
        .size = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: \"hello, world!\"",
        .max_entries = TEST_MAX_ENTRIES,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_STR, .val.str = "hello, world!"},
            },
        .size = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: 10 # Inline comment",
        .max_entries = TEST_MAX_ENTRIES,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_INT, .val.int_ = 10},
            },
        .size = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: -1",
        .max_entries = TEST_MAX_ENTRIES,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_INT, .val.int_ = -1},
            },
        .size = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: true",
        .max_entries = TEST_MAX_ENTRIES,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_BOOL, .val.bool_ = true},
            },
        .size = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: false",
        .max_entries = TEST_MAX_ENTRIES,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_BOOL, .val.bool_ = false},
            },
        .size = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 1)",
        .max_entries = TEST_MAX_ENTRIES,
        .entries =
            (CfgEntry[]){
                {.key = "key",
                 .type = CFG_TYPE_COLOR,
                 .val.color = (CfgColor){.r = 255, .g = 255, .b = 255, .a = 255}},
            },
        .size = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: 0.5",
        .max_entries = TEST_MAX_ENTRIES,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_FLOAT, .val.float_ = 0.5},
            },
        .size = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key_: true",
        .max_entries = TEST_MAX_ENTRIES,
        .entries =
            (CfgEntry[]){
                {.key = "key_", .type = CFG_TYPE_BOOL, .val.bool_ = true},
            },
        .size = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key.: true",
        .max_entries = TEST_MAX_ENTRIES,
        .entries =
            (CfgEntry[]){
                {.key = "key.", .type = CFG_TYPE_BOOL, .val.bool_ = true},
            },
        .size = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "a: true\nb:true",
        .max_entries = TEST_MAX_ENTRIES,
        .entries =
            (CfgEntry[]){
                {.key = "a", .type = CFG_TYPE_BOOL, .val.bool_ = true},
                {.key = "b", .type = CFG_TYPE_BOOL, .val.bool_ = true},
            },
        .size = 2,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: 1.",
        .max_entries = TEST_MAX_ENTRIES,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_FLOAT, .val.float_ = 1.0},
            },
        .size = 1,
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "!",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "missing key",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        // The key must be longer than CFG_MAX_KEY
        .src = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "key too long",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "':' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key!",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "':' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key  :",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key:",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key:  ",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key:\n",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key:  \n",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: @\n",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "invalid value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: \"",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: \"\x80",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: \"hello",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: \"hello\x80",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        // The value must be longer than CFG_MAX_VAL
        .src = "key: \"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\"",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "value too long",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: 10x",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "unexpected character 'x'",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: -",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "invalid value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: t",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "invalid literal",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: f",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "invalid literal",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: x",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "invalid literal",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "',' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "'(' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba x",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "'(' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: r",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "invalid literal",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(0.5",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(-1",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(-1",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(256",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, -1)",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "alpha must be in range (0, 1)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 2)",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "alpha must be in range (0, 1)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 1",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "')' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, x",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "number expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(x",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "number expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255 x",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "',' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 1 x",
        .max_entries = TEST_MAX_ENTRIES,
        .err = "')' expected",
    },
};

static const TestCase loading_test_cases[] = {
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "sample.cfg",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "sample.c",
        .err = "invalid file extension",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "sample2.cfg",
        .err = "failed to open the file",
    },
};

static FILE *stream;
static Scoreboard scoreboard;

static void
reset_scoreboard()
{
    scoreboard.total = 0;
    scoreboard.passed = 0;
    scoreboard.failed = 0;
}

static bool
assert_eq_entry(const CfgEntry *expected, const CfgEntry *actual)
{
    if (expected->type != actual->type) {
        static const char fmt[] = "Type mismatch in entry \"%s\" between "
                                  " [expected] and [actual]\n";
        fprintf(stream, fmt, expected->key);
        return false;
    }

    if (strcmp(expected->key, actual->key) != 0) {
        static const char fmt[] = "Key mismatch in entry \"%s\" between "
                                  "[expected] and [actual]\n";
        fprintf(stream, fmt, expected->key);
        return false;
    }

    switch (expected->type) {
    case CFG_TYPE_STR:
        if (!strcmp(expected->val.str, actual->val.str))
            return true;
        break;

    case CFG_TYPE_INT:
        if (expected->val.int_ == actual->val.int_)
            return true;
        break;

    case CFG_TYPE_FLOAT:
        if (expected->val.float_ == actual->val.float_)
            return true;
        break;

    case CFG_TYPE_BOOL:
        if (expected->val.bool_ == actual->val.bool_)
            return true;
        break;

    case CFG_TYPE_COLOR:
        if (!memcmp(&expected->val.color, &actual->val.color, sizeof(CfgColor)))
            return true;
        break;

    default:
        fprintf(stderr, "FATAL: unknown CfgEntry type\n");
        exit(1);
    }

    static const char fmt[] = "Value mismatch in entry \"%s\" between "
                              "[expected] and [actual].\n";
    fprintf(stream, fmt, expected->key);
    return false;
}

static bool
assert_eq_entries(const TestCase expected, const CfgEntry *actual_entries)
{
    for (int i = 0; i < expected.size; i++) {
        if (!assert_eq_entry(&expected.entries[i], &actual_entries[i]))
            return false;
    }

    return true;
}

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
run_parsing_test_case(TestCase tc)
{
    Cfg cfg;
    CfgEntry entries[TEST_MAX_ENTRIES];

    assert(tc.max_entries <= TEST_MAX_ENTRIES);
    cfg_init(&cfg, entries, tc.max_entries);

    CfgError err;
    int res = cfg_parse(tc.src, strlen(tc.src), &cfg, &err);

    switch (tc.type) {
    case TC_SUCC:
        if (res != 0) {
            // SUCCESS CASE - FAILED (failed parsing)
            cfg_fprint_error(stream, &err);
            log_result(tc, true);
            scoreboard.failed++;
        } else {
            if (tc.size != cfg.size) {
                // SUCCESS CASE - FAILED (entries size mismatch)
                fprintf(stream, "Size mismatch between [expected] and [actual]\n");
                log_result(tc, true);
                scoreboard.failed++;
            } else if (!assert_eq_entries(tc, cfg.entries)) {
                // SUCCESS CASE - FAILED (entries mismatch)
                log_result(tc, true);
                scoreboard.failed++;
            } else {
                // SUCCESS CASE - PASSED
                log_result(tc, false);
                scoreboard.passed++;
            }
        }
        break;

    case TC_ERR:
        if (res == 0) {
            // ERROR CASE - FAILED (successful parsing)
            fprintf(stream, "Error case was parsed successfully\n");
            log_result(tc, true);
            scoreboard.failed++;
        } else {
            if (strcmp(tc.err, err.msg) != 0) {
                // ERROR CASE - FAILED (error message mismatch)
                fprintf(stream, "Error message mismatch between "
                                "[expected] and [actual]\n");
                log_result(tc, false);
                scoreboard.failed++;
            } else {
                // ERROR CASE - PASSED
                log_result(tc, false);
                scoreboard.passed++;
            }
        }
        break;
    }
}

static void
run_loading_test_case(TestCase tc)
{
    Cfg cfg;
    CfgEntry entries[TEST_MAX_ENTRIES];

    cfg_init(&cfg, entries, TEST_MAX_ENTRIES);

    CfgError err;
    int res = cfg_load(tc.src, &cfg, &err);

    switch (tc.type) {
    case TC_SUCC:
        if (res != 0) {
            // SUCCESS CASE - FAILED
            cfg_fprint_error(stream, &err);
            log_result(tc, true);
            scoreboard.failed++;
        } else {
            // SUCCESS CASE - PASSED
            log_result(tc, false);
            scoreboard.passed++;
        }
        break;

    case TC_ERR:
        if (res == 0) {
            // ERROR CASE - FAILED (successful parsing)
            fprintf(stream, "Error case was parsed successfully\n");
            log_result(tc, true);
            scoreboard.failed++;
        } else {
            if (strcmp(tc.err, err.msg) != 0) {
                // ERROR CASE - FAILED (error message mismatch)
                cfg_fprint_error(stderr, &err);
                fprintf(stream, "ERROR   CASE - " RED "FAILED " RESET "(%s:%d)\n",
                        __FILE__, __LINE__);
                scoreboard.failed++;
            } else {
                // ERROR CASE - PASSED
                fprintf(stream, "ERROR   CASE - " GREEN "PASSED " RESET "(%s:%d)\n",
                        __FILE__, __LINE__);
                scoreboard.passed++;
            }
        }
        break;
    }
}

int
main(void)
{
#ifdef LOGFILE
    stream = fopen("log.txt", "w");
    if (!stream) {
        fprintf(stderr, "FATAL: Failed to open log file\n");
        exit(1);
    }
#else
    stream = stdout;
#endif

    reset_scoreboard();

    fprintf(stdout, "PARSING:\n");
    int num_parsing_tcs = sizeof(parsing_test_cases) / sizeof(parsing_test_cases[0]);
    scoreboard.total += num_parsing_tcs;
    for (int i = 0; i < num_parsing_tcs; i++) {
        run_parsing_test_case(parsing_test_cases[i]);
    }

    fprintf(stdout, "LOADING:\n");
    int num_loading_tcs = sizeof(loading_test_cases) / sizeof(loading_test_cases[0]);
    scoreboard.total += num_loading_tcs;
    for (int i = 0; i < num_loading_tcs; i++) {
        run_loading_test_case(loading_test_cases[i]);
    }

    fprintf(stream, "Total: %d Passed: %d Failed: %d\n", scoreboard.total,
            scoreboard.passed, scoreboard.failed);

#ifdef LOGFILE
    fclose(stream);
#endif
    return 0;
}

// Error list:

// missing key
// missing value

// key too long
// value too long

// invalid value
// invalid literal

// closing '\"' expected
// number expected
// '(' expected
// ')' expected
// ',' expected
// ':' expected

// red, blue and green must be integers in range (0, 255)
// alpha must be in range (0, 1)

// unexpected character '%c'
