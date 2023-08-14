#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "test.h"
#include "test_parse.h"

typedef struct {
    enum { TC_SUCC, TC_ERR } type;
    int line;
    const char *src;
    int capacity;
    // TC_SUCC
    CfgEntry *entries;  // Expected entries
    int count;          // Expected count of [entries]
    // TC_ERR
    const char *err;  // Expected error message
} TestCase;

static const TestCase test_cases[] = {
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "",
        .capacity = TEST_CAPACITY,
        .entries = (CfgEntry[]){},
        .count = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = " ",
        .capacity = TEST_CAPACITY,
        .entries = (CfgEntry[]){},
        .count = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "#",
        .capacity = TEST_CAPACITY,
        .entries = (CfgEntry[]){},
        .count = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "#\n",
        .capacity = TEST_CAPACITY,
        .entries = (CfgEntry[]){},
        .count = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "x",
        .capacity = 0,
        .entries = (CfgEntry[]){},
        .count = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: \"hello, world!\"",
        .capacity = TEST_CAPACITY,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_STR, .val.str = "hello, world!"},
            },
        .count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: 10 # Inline comment",
        .capacity = TEST_CAPACITY,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_INT, .val.int_ = 10},
            },
        .count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: -1",
        .capacity = TEST_CAPACITY,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_INT, .val.int_ = -1},
            },
        .count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: true",
        .capacity = TEST_CAPACITY,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_BOOL, .val.bool_ = true},
            },
        .count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: false",
        .capacity = TEST_CAPACITY,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_BOOL, .val.bool_ = false},
            },
        .count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 1)",
        .capacity = TEST_CAPACITY,
        .entries =
            (CfgEntry[]){
                {.key = "key",
                 .type = CFG_TYPE_COLOR,
                 .val.color = (CfgColor){.r = 255, .g = 255, .b = 255, .a = 255}},
            },
        .count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: 0.5",
        .capacity = TEST_CAPACITY,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_FLOAT, .val.float_ = 0.5},
            },
        .count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key_: true",
        .capacity = TEST_CAPACITY,
        .entries =
            (CfgEntry[]){
                {.key = "key_", .type = CFG_TYPE_BOOL, .val.bool_ = true},
            },
        .count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key.: true",
        .capacity = TEST_CAPACITY,
        .entries =
            (CfgEntry[]){
                {.key = "key.", .type = CFG_TYPE_BOOL, .val.bool_ = true},
            },
        .count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "a: true\nb:true",
        .capacity = TEST_CAPACITY,
        .entries =
            (CfgEntry[]){
                {.key = "a", .type = CFG_TYPE_BOOL, .val.bool_ = true},
                {.key = "b", .type = CFG_TYPE_BOOL, .val.bool_ = true},
            },
        .count = 2,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: 1.",
        .capacity = TEST_CAPACITY,
        .entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_FLOAT, .val.float_ = 1.0},
            },
        .count = 1,
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "!",
        .capacity = TEST_CAPACITY,
        .err = "missing key",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        // The key must be longer than CFG_MAX_KEY
        .src = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
        .capacity = TEST_CAPACITY,
        .err = "key too long",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key",
        .capacity = TEST_CAPACITY,
        .err = "':' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key!",
        .capacity = TEST_CAPACITY,
        .err = "':' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key  :",
        .capacity = TEST_CAPACITY,
        .err = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key:",
        .capacity = TEST_CAPACITY,
        .err = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key:  ",
        .capacity = TEST_CAPACITY,
        .err = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key:\n",
        .capacity = TEST_CAPACITY,
        .err = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key:  \n",
        .capacity = TEST_CAPACITY,
        .err = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: @\n",
        .capacity = TEST_CAPACITY,
        .err = "invalid value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: \"",
        .capacity = TEST_CAPACITY,
        .err = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: \"\x80",
        .capacity = TEST_CAPACITY,
        .err = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: \"hello",
        .capacity = TEST_CAPACITY,
        .err = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: \"hello\x80",
        .capacity = TEST_CAPACITY,
        .err = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        // The value must be longer than CFG_MAX_VAL
        .src = "key: \"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\"",
        .capacity = TEST_CAPACITY,
        .err = "value too long",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: 10x",
        .capacity = TEST_CAPACITY,
        .err = "unexpected character 'x'",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: -",
        .capacity = TEST_CAPACITY,
        .err = "invalid value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: t",
        .capacity = TEST_CAPACITY,
        .err = "invalid literal",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: f",
        .capacity = TEST_CAPACITY,
        .err = "invalid literal",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: x",
        .capacity = TEST_CAPACITY,
        .err = "invalid literal",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(",
        .capacity = TEST_CAPACITY,
        .err = "',' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba",
        .capacity = TEST_CAPACITY,
        .err = "'(' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba x",
        .capacity = TEST_CAPACITY,
        .err = "'(' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: r",
        .capacity = TEST_CAPACITY,
        .err = "invalid literal",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(0.5",
        .capacity = TEST_CAPACITY,
        .err = "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(-1",
        .capacity = TEST_CAPACITY,
        .err = "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(-1",
        .capacity = TEST_CAPACITY,
        .err = "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(256",
        .capacity = TEST_CAPACITY,
        .err = "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, -1)",
        .capacity = TEST_CAPACITY,
        .err = "alpha must be in range (0, 1)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 2)",
        .capacity = TEST_CAPACITY,
        .err = "alpha must be in range (0, 1)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 1",
        .capacity = TEST_CAPACITY,
        .err = "')' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, x",
        .capacity = TEST_CAPACITY,
        .err = "number expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(x",
        .capacity = TEST_CAPACITY,
        .err = "number expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255 x",
        .capacity = TEST_CAPACITY,
        .err = "',' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 1 x",
        .capacity = TEST_CAPACITY,
        .err = "')' expected",
    },
};

static FILE *stream;

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
    for (int i = 0; i < expected.count; i++) {
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
run_test_case(TestCase tc, Scoreboard *scoreboard)
{
    Cfg cfg;
    CfgEntry entries[TEST_CAPACITY];

    assert(tc.capacity <= TEST_CAPACITY);
    cfg_init(&cfg, entries, tc.capacity);

    CfgError err;
    int res = cfg_parse(tc.src, strlen(tc.src), &cfg, &err);

    switch (tc.type) {
    case TC_SUCC:
        if (res != 0) {
            // SUCCESS CASE - FAILED (failed parsing)
            cfg_fprint_error(stream, &err);
            log_result(tc, true);
            scoreboard->failed++;
        } else {
            if (tc.count != cfg.count) {
                // SUCCESS CASE - FAILED (entries count mismatch)
                fprintf(stream, "Count mismatch between [expected] and [actual]\n");
                log_result(tc, true);
                scoreboard->failed++;
            } else if (!assert_eq_entries(tc, cfg.entries)) {
                // SUCCESS CASE - FAILED (entries mismatch)
                log_result(tc, true);
                scoreboard->failed++;
            } else {
                // SUCCESS CASE - PASSED
                log_result(tc, false);
                scoreboard->passed++;
            }
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
                fprintf(stream, "Error message mismatch between "
                                "[expected] and [actual]\n");
                log_result(tc, false);
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
run_parse_tests(Scoreboard *scoreboard, FILE *stream_)
{
    stream = stream_;

    int total = sizeof(test_cases) / sizeof(test_cases[0]);
    scoreboard->total += total;

    for (int i = 0; i < total; i++) {
        run_test_case(test_cases[i], scoreboard);
    }
}
