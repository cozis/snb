#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../config.h"
#include "test.h"
#include "test_get.h"

typedef struct {
    CfgValType type;
    int line;
    Cfg cfg;
    CfgVal default_;
    CfgVal expected;
} TestCase;

static const TestCase test_cases[] = {
    {
        .type = CFG_TYPE_STR,
        .line = __LINE__,
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_STR,
                            .val.str = "hello, world!",
                        },
                    },
                .count = 1,
            },
        .expected = {.str = "hello, world!"},
    },
    {
        .type = CFG_TYPE_BOOL,
        .line = __LINE__,
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_BOOL,
                            .val.bool_ = true,
                        },
                    },
                .count = 1,
            },
        .expected = {.bool_ = true},
    },
    {
        .type = CFG_TYPE_INT,
        .line = __LINE__,
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_INT,
                            .val.int_ = 10,
                        },
                    },
                .count = 1,
            },
        .expected = {.int_ = 10},
    },
    {
        .type = CFG_TYPE_FLOAT,
        .line = __LINE__,
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_FLOAT,
                            .val.float_ = 0.5,
                        },
                    },
                .count = 1,
            },
        .expected = {.float_ = 0.5},
    },
    {
        .type = CFG_TYPE_COLOR,
        .line = __LINE__,
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_COLOR,
                            .val.color =
                                (CfgColor){
                                    .r = 255,
                                    .g = 255,
                                    .b = 255,
                                    .a = 255,
                                },
                        },
                    },
                .count = 1,
            },
        .expected = {.color =
                         (CfgColor){
                             .r = 255,
                             .g = 255,
                             .b = 255,
                             .a = 255,
                         }},
    },
    {
        .type = CFG_TYPE_BOOL,
        .line = __LINE__,
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries = (CfgEntry[]){},
                .count = 0,
            },
        .expected = true,
        .default_ = true,
    },
    {
        .type = CFG_TYPE_INT,
        .line = __LINE__,
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key2",
                            .type = CFG_TYPE_INT,
                            .val.int_ = 10,
                        },
                    },
                .count = 1,
            },
        .expected = true,
        .default_ = true,
    },
    {
        .type = CFG_TYPE_BOOL,
        .line = __LINE__,
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_INT,
                            .val.int_ = 10,
                        },
                    },
                .count = 1,
            },
        .expected = true,
        .default_ = true,
    },
};

static FILE *stream;

static void
log_result(TestCase tc, bool failed)
{
    if (failed) {
        fprintf(stream, "SUCCESS CASE - " RED "FAILED " RESET "(%s:%d)\n", __FILE__,
                tc.line);
    } else {
        fprintf(stream, "SUCCESS CASE - " GREEN "PASSED " RESET "(%s:%d)\n",
                __FILE__, tc.line);
    }
}

static void
run_test_case(TestCase tc, Scoreboard *scoreboard)
{
    bool success = false;

    switch (tc.type) {
    case CFG_TYPE_STR:
        char *str = cfg_get_str(tc.cfg, "key", tc.default_.str);
        success = !strcmp(tc.expected.str, str);
        break;
    case CFG_TYPE_BOOL:
        bool bool_ = cfg_get_bool(tc.cfg, "key", tc.default_.bool_);
        success = tc.expected.bool_ == bool_;
        break;
    case CFG_TYPE_INT:
        int int_ = cfg_get_int(tc.cfg, "key", tc.default_.int_);
        success = tc.expected.int_ == int_;
        break;
    case CFG_TYPE_FLOAT:
        float float_ = cfg_get_float(tc.cfg, "key", tc.default_.float_);
        success = tc.expected.float_ == float_;
        break;
    case CFG_TYPE_COLOR:
        CfgColor color = cfg_get_color(tc.cfg, "key", tc.default_.color);
        success = !memcmp(&tc.expected.color, &color, sizeof(CfgColor));
        break;
    }

    if (!success) {
        log_result(tc, true);
        scoreboard->failed++;
    } else {
        log_result(tc, false);
        scoreboard->passed++;
    }
}

void
run_get_tests(Scoreboard *scoreboard, FILE *stream_)
{
    stream = stream_;

    int total = sizeof(test_cases) / sizeof(test_cases[0]);
    scoreboard->total += total;

    for (int i = 0; i < total; i++) {
        run_test_case(test_cases[i], scoreboard);
    }
}
