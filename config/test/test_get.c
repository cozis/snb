#include <stdio.h>
#include <string.h>

#include "../config.h"
#include "test.h"
#include "test_get.h"

typedef enum {
    TC_GET_STR,
    TC_GET_BOOL,
    TC_GET_INT,
    TC_GET_INT_MIN,
    TC_GET_INT_MAX,
    TC_GET_INT_RANGE,
    TC_GET_FLOAT,
    TC_GET_FLOAT_MIN,
    TC_GET_FLOAT_MAX,
    TC_GET_FLOAT_RANGE,
    TC_GET_COLOR
} TestCaseType;

typedef union {
    int int_;
    float float_;
} MinMax;

typedef struct {
    TestCaseType type;
    int line;
    char *key;
    Cfg cfg;
    CfgVal default_;
    MinMax min;
    MinMax max;
    CfgVal expected;
} TestCase;

static const TestCase test_cases[] = {
    {
        .type = TC_GET_STR,
        .line = __LINE__,
        .key = "key",
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
        .expected.str = "hello, world!",
    },
    {
        .type = TC_GET_BOOL,
        .line = __LINE__,
        .key = "key",
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
        .expected.bool_ = true,
    },
    {
        .type = TC_GET_INT,
        .line = __LINE__,
        .key = "key",
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
        .expected.int_ = 10,
    },
    {
        .type = TC_GET_FLOAT,
        .line = __LINE__,
        .key = "key",
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
        .expected.float_ = 0.5,
    },
    {
        .type = TC_GET_COLOR,
        .line = __LINE__,
        .key = "key",
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
        .expected.color =
            (CfgColor){
                .r = 255,
                .g = 255,
                .b = 255,
                .a = 255,
            },
    },
    {
        .type = TC_GET_BOOL,
        .line = __LINE__,
        .key = "key",
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries = (CfgEntry[]){},
                .count = 0,
            },
        .expected.bool_ = true,
        .default_.bool_ = true,
    },
    {
        .type = TC_GET_INT,
        .line = __LINE__,
        .key = "key",
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "__key__",
                            .type = CFG_TYPE_INT,
                            .val.int_ = 10,
                        },
                    },
                .count = 1,
            },
        .expected.int_ = 5,
        .default_.int_ = 5,
    },
    {
        .type = TC_GET_BOOL,
        .line = __LINE__,
        .key = "key",
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
        .expected.bool_ = true,
        .default_.bool_ = true,
    },
    {
        .type = TC_GET_INT_MIN,
        .line = __LINE__,
        .key = "key",
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
        .expected.int_ = 10,
        .default_.int_ = 5,
        .min.int_ = 1,
    },
    {
        .type = TC_GET_INT_MIN,
        .line = __LINE__,
        .key = "key",
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
        .expected.int_ = 15,
        .default_.int_ = 15,
        .min.int_ = 11,
    },
    {
        .type = TC_GET_INT_MAX,
        .line = __LINE__,
        .key = "key",
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
        .expected.int_ = 10,
        .default_.int_ = 5,
        .max.int_ = 15,
    },
    {
        .type = TC_GET_INT_MAX,
        .line = __LINE__,
        .key = "key",
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
        .expected.int_ = 1,
        .default_.int_ = 1,
        .max.int_ = 5,
    },
    {
        .type = TC_GET_INT_RANGE,
        .line = __LINE__,
        .key = "key",
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
        .expected.int_ = 10,
        .default_.int_ = 5,
        .min.int_ = 1,
        .max.int_ = 15,
    },
    {
        .type = TC_GET_INT_RANGE,
        .line = __LINE__,
        .key = "key",
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_INT,
                            .val.int_ = 0,
                        },
                    },
                .count = 1,
            },
        .expected.int_ = 5,
        .default_.int_ = 5,
        .min.int_ = 1,
        .max.int_ = 9,
    },
    {
        .type = TC_GET_INT_RANGE,
        .line = __LINE__,
        .key = "key",
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
        .expected.int_ = 5,
        .default_.int_ = 5,
        .min.int_ = 1,
        .max.int_ = 9,
    },
    {
        .type = TC_GET_FLOAT_MIN,
        .line = __LINE__,
        .key = "key",
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_FLOAT,
                            .val.float_ = 10.0,
                        },
                    },
                .count = 1,
            },
        .expected.float_ = 10.0,
        .default_.float_ = 5.0,
        .min.float_ = 1.0,
    },
    {
        .type = TC_GET_FLOAT_MIN,
        .line = __LINE__,
        .key = "key",
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_FLOAT,
                            .val.float_ = 10.0,
                        },
                    },
                .count = 1,
            },
        .expected.float_ = 15.0,
        .default_.float_ = 15.0,
        .min.float_ = 11,
    },
    {
        .type = TC_GET_FLOAT_MAX,
        .line = __LINE__,
        .key = "key",
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_FLOAT,
                            .val.float_ = 10.0,
                        },
                    },
                .count = 1,
            },
        .expected.float_ = 10.0,
        .default_.float_ = 5.0,
        .max.float_ = 15.0,
    },
    {
        .type = TC_GET_FLOAT_MAX,
        .line = __LINE__,
        .key = "key",
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_FLOAT,
                            .val.float_ = 10.0,
                        },
                    },
                .count = 1,
            },
        .expected.float_ = 1.0,
        .default_.float_ = 1.0,
        .max.float_ = 5.0,
    },
    {
        .type = TC_GET_FLOAT_RANGE,
        .line = __LINE__,
        .key = "key",
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_FLOAT,
                            .val.float_ = 10.0,
                        },
                    },
                .count = 1,
            },
        .expected.float_ = 10.0,
        .default_.float_ = 5.0,
        .min.float_ = 1.0,
        .max.float_ = 15.0,
    },
    {
        .type = TC_GET_FLOAT_RANGE,
        .line = __LINE__,
        .key = "key",
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_FLOAT,
                            .val.float_ = 0,
                        },
                    },
                .count = 1,
            },
        .expected.float_ = 5.0,
        .default_.float_ = 5.0,
        .min.float_ = 1.0,
        .max.float_ = 9.0,
    },
    {
        .type = TC_GET_FLOAT_RANGE,
        .line = __LINE__,
        .key = "key",
        .cfg =
            (Cfg){
                .capacity = TEST_CAPACITY,
                .entries =
                    (CfgEntry[]){
                        {
                            .key = "key",
                            .type = CFG_TYPE_FLOAT,
                            .val.float_ = 10.0,
                        },
                    },
                .count = 1,
            },
        .expected.float_ = 5.0,
        .default_.float_ = 5.0,
        .min.float_ = 1.0,
        .max.float_ = 9.0,
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
    int int_;
    float float_;

    switch (tc.type) {
    case TC_GET_STR:
        char *str = cfg_get_str(tc.cfg, tc.key, tc.default_.str);
        success = !strcmp(tc.expected.str, str);
        break;
    case TC_GET_BOOL:
        bool bool_ = cfg_get_bool(tc.cfg, tc.key, tc.default_.bool_);
        success = tc.expected.bool_ == bool_;
        break;
    case TC_GET_INT:
        int_ = cfg_get_int(tc.cfg, tc.key, tc.default_.int_);
        success = tc.expected.int_ == int_;
        break;
    case TC_GET_INT_MIN:
        int_ = cfg_get_int_min(tc.cfg, tc.key, tc.default_.int_, tc.min.int_);
        success = tc.expected.int_ == int_;
        break;
    case TC_GET_INT_MAX:
        int_ = cfg_get_int_max(tc.cfg, tc.key, tc.default_.int_, tc.max.int_);
        success = tc.expected.int_ == int_;
        break;
    case TC_GET_INT_RANGE:
        int_ = cfg_get_int_range(tc.cfg, tc.key, tc.default_.int_, tc.min.int_,
                                 tc.max.int_);
        success = tc.expected.int_ == int_;
        break;
    case TC_GET_FLOAT:
        float_ = cfg_get_float(tc.cfg, tc.key, tc.default_.float_);
        success = tc.expected.float_ == float_;
        break;
    case TC_GET_FLOAT_MIN:
        float_ =
            cfg_get_float_min(tc.cfg, tc.key, tc.default_.float_, tc.min.float_);
        success = tc.expected.float_ == float_;
        break;
    case TC_GET_FLOAT_MAX:
        float_ =
            cfg_get_float_max(tc.cfg, tc.key, tc.default_.float_, tc.max.float_);
        success = tc.expected.float_ == float_;
        break;
    case TC_GET_FLOAT_RANGE:
        float_ = cfg_get_float_range(tc.cfg, tc.key, tc.default_.float_,
                                     tc.min.float_, tc.max.float_);
        success = tc.expected.float_ == float_;
        break;
    case TC_GET_COLOR:
        CfgColor color = cfg_get_color(tc.cfg, tc.key, tc.default_.color);
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
