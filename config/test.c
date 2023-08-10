#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#define COLORS
#define MAX_ENTRIES 64

#ifdef COLORS
#define RED "\e[1;31m"
#define GREEN "\e[1;32m"
#define RESET "\e[0m"
#else
#define RED
#define GREEN
#define RESET
#endif

typedef struct {
    enum { TC_SUCC, TC_ERR } type;
    const char *src;
    union {
        const char *err;
        Cfg cfg;
    } exp;
} TestCase;

// clang-format off
static const TestCase cases[] = {
    // ** SUCCESS CASES ** //
    {
        // Test: average config file
        .type = TC_SUCC,
        .src = "# A sample config file for a generic text editor\n"
               "font: \"JetBrainsMono Nerd Font\"\n"
               "font.size: 14\n"
               "zoom: 1.5\n"
               "line_numbers: true\n"
               "bg.color: rgba(255, 255, 255, 1)",
        .exp.cfg = {
            .entries = (CfgEntry[]){
                { .key = "font", .type = CFG_TYPE_STR, .val.str = "JetBrainsMono Nerd Font" },
                { .key = "font.size", .type = CFG_TYPE_INT, .val.int_ = 14 },
                { .key = "zoom", .type = CFG_TYPE_FLOAT, .val.float_ = 1.5 },
                { .key = "line_numbers", .type = CFG_TYPE_BOOL, .val.bool_ = true },
                { .key = "bg.color", .type = CFG_TYPE_COLOR, .val.color = {.r = 255, .g = 255, .b = 255, .a = 255} }
            },
            .max_entries = MAX_ENTRIES,
            .size = 5
        }
    },
    {
        // Test: Unusual keys
        .type = TC_SUCC,
        .src = "__k__e__y__: true\n"
                "..k..e..y..: true\n"
                "___: true\n"
                "...: true\n"
                ".: true\n"
                "_: true",
        .exp.cfg = {
            .entries = (CfgEntry[]){
                { .key = "__k__e__y__", .type = CFG_TYPE_BOOL, .val.bool_ = true },
                { .key = "..k..e..y..",  .type = CFG_TYPE_BOOL, .val.bool_ = true },
                { .key = "___", .type = CFG_TYPE_BOOL, .val.bool_ = true },
                { .key = "...", .type = CFG_TYPE_BOOL, .val.bool_ = true },
                { .key = ".", .type = CFG_TYPE_BOOL, .val.bool_ = true },
                { .key = "_", .type = CFG_TYPE_BOOL, .val.bool_ = true },
            },
            .max_entries = MAX_ENTRIES,
            .size = 6
        }
    },
    {
        // Test: Unusual values
        .type = TC_SUCC,
        .src = "a: \"`~!@#$^&*()-_=+[]{}|;:',.<>/?\"\n"
               "  b:  rgba  (  0  ,  255  ,  127  ,  0.005  )  ",
        .exp.cfg = {
            .entries = (CfgEntry[]){
                { .key = "a", .type = CFG_TYPE_STR, .val.str = "`~!@#$^&*()-_=+[]{}|;:',.<>/?" },
                { .key = "b", .type = CFG_TYPE_COLOR, .val.color = {.r = 0, .g = 255, .b = 127, .a = 1} },
            },
            .max_entries = MAX_ENTRIES,
            .size = 2
        }
    },
    {
        // Test: Inline comments
        .type = TC_SUCC,
        .src =  "a: true # Inline comment\n"
                "b: true # Inline comment",
        .exp.cfg = {
            .entries = (CfgEntry[]){
                { .key = "a", .type = CFG_TYPE_BOOL, .val.bool_ = true },
                { .key = "b", .type = CFG_TYPE_BOOL, .val.bool_ = true },
            },
            .max_entries = MAX_ENTRIES,
            .size = 2
        }
    },
    {
        // Test: Integers out of range (wrap around)
        .type = TC_SUCC,
        .src = "a: 21474836470\n"
               "b: -21474836470",
        .exp.cfg = {
            .entries = (CfgEntry[]){
                { .key = "a", .type = CFG_TYPE_INT, .val.int_ = -10 },
                { .key = "b", .type = CFG_TYPE_INT, .val.int_ = 10 },
            },
            .max_entries = MAX_ENTRIES,
            .size = 2
        }
    },
    {
        // Test: Leading and trailing newlines
        .type = TC_SUCC,
        .src = "\n\na: true\n\n",
        .exp.cfg = {
            .entries = (CfgEntry[]){
                { .key = "a", .type = CFG_TYPE_BOOL, .val.bool_ = true }
            },
            .max_entries = MAX_ENTRIES,
            .size = 1
        }
    },
    {
        // Test: Leading and trailing spaces
        .type = TC_SUCC,
        .src = "  \t  a    :    true  \t  ",
        .exp.cfg = {
            .entries = (CfgEntry[]){
                { .key = "a", .type = CFG_TYPE_BOOL, .val.bool_ = true }
            },
            .max_entries = MAX_ENTRIES,
            .size = 1
        }
    },
    {
        // Test: No spaces
        .type = TC_SUCC,
        .src = "a:true",
        .exp.cfg = {
            .entries = (CfgEntry[]){
                { .key = "a", .type = CFG_TYPE_BOOL, .val.bool_ = true }
            },
            .max_entries = MAX_ENTRIES,
            .size = 1
        }
    },
    // FIXME: don't really know how to test this...
    // {
    //     // Test: Empty
    //     .type = TC_SUCC,
    //     .src = "",
    //     .exp.cfg = {
    //         .entries = {0},
    //         .max_entries = MAX_ENTRIES,
    //         .size = 1
    //     }
    // },
    // ** ERROR CASES ** //
    {
        // Test: Invalid key
        .type = TC_ERR,
        .src = "a-b: 1",
        .exp.err = "':' expected"
    },
    {
        // Test: Invalid value
        .type = TC_ERR,
        .src = "a:~",
        .exp.err = "invalid value"
    },
    {
        // Test: Key too long
        .type = TC_ERR,
        .src = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa:true",
        .exp.err = "key too long"
    },
    {
        // Test: Value too long
        .type = TC_ERR,
        .src = "a: \"                                                                 \"",
        .exp.err = "value too long"
    },
    {
        // Test: Missing key
        .type = TC_ERR,
        .src = ":true",
        .exp.err = "missing key"
    },
    {
        // Test: Missing value
        .type = TC_ERR,
        .src = "a:",
        .exp.err = "missing value"
    },
    {
        // Test: Missing ':'
        .type = TC_ERR,
        .src = "a 1",
        .exp.err = "':' expected"
    },
    {
        // Test: Missing opening '"'
        .type = TC_ERR,
        .src = "a: lorem ipsum\"",
        .exp.err = "invalid literal"
    },
    {
        // Test: Missing closing '"'
        .type = TC_ERR,
        .src = "a: \"lorem ipsum",
        .exp.err = "closing '\"' expected"
    },
    {
        // Test: Missing '('
        .type = TC_ERR,
        .src = "a: rgba 255, 255, 255, 1)",
        .exp.err = "'(' expected"
    },
    {
        // Test: Missing ')'
        .type = TC_ERR,
        .src = "a: rgba(255, 255, 255, 1",
        .exp.err = "')' expected"
    },
    {
        // Test: Missing ','
        .type = TC_ERR,
        .src = "a: rgba(255 255, 255, 1)",
        .exp.err = "',' expected"
    },
    {
        // Test: Newline before colon
        .type = TC_ERR,
        .src = "a\n:1",
        .exp.err = "':' expected"
    },
    {
        // Test: Newline before value
        .type = TC_ERR,
        .src = "a:\n1",
        .exp.err = "missing value"
    },
    {
        // Test: Newline inside a string
        .type = TC_ERR,
        .src = "a: \"lorem\nipsum\"",
        .exp.err = "closing '\"' expected"
    },
    {
        // Test: Unexpected character [1]
        .type = TC_ERR,
        .src = "a: 1 true",
        .exp.err = "unexpected character 't'"
    },
    {
        // Test: Unexpected character [2]
        .type = TC_ERR,
        .src = "a: falsex",
        .exp.err = "unexpected character 'x'"
    },
    {
        // Test: Red out of range
        .type = TC_ERR,
        .src = "a: rgba(256, 255, 255, 1)",
        .exp.err = "red, blue and green must be integers in range (0, 255)"
    },
    {
        // Test: Green out of range
        .type = TC_ERR,
        .src = "a: rgba(255, -1, 255, 1)",
        .exp.err = "red, blue and green must be integers in range (0, 255)"
    },
    {
        // Test: Blue not an integer
        .type = TC_ERR,
        .src = "a: rgba(255, 255, 0.5, 1)",
        .exp.err = "red, blue and green must be integers in range (0, 255)"
    },
    {
        // Test: Alpha out of range [1]
        .type = TC_ERR,
        .src = "a: rgba(255, 255, 255, 1.1)",
        .exp.err = "alpha must be in range (0, 1)"
    },
    {
        // Test: Alpha out of range [2]
        .type = TC_ERR,
        .src = "a: rgba(255, 255, 255, -0.1)",
        .exp.err = "alpha must be in range (0, 1)"
    },
  };
// clang-format on

bool
compare_entries(const CfgEntry *expected, const CfgEntry *actual)
{
    if (expected->type != actual->type)
        return false;

    if (strcmp(expected->key, actual->key) != 0)
        return false;

    switch (expected->type) {
    case CFG_TYPE_STR:
        return !strcmp(expected->val.str, actual->val.str);

    case CFG_TYPE_INT:
        return expected->val.int_ == actual->val.int_;

    case CFG_TYPE_FLOAT:
        return expected->val.float_ == actual->val.float_;

    case CFG_TYPE_BOOL:
        return expected->val.bool_ == actual->val.bool_;

    case CFG_TYPE_COLOR:
        return !memcmp(&expected->val.color, &actual->val.color, sizeof(CfgColor));

    default:
        return false;
    }

    return false;
}

bool
compare_configs(const Cfg expected, const Cfg actual)
{
    if (expected.max_entries != actual.max_entries)
        return false;

    if (expected.size != actual.size)
        return false;

    if (expected.entries->type != actual.entries->type)
        return false;

    for (int i = 0; i < expected.size; i++) {
        if (!compare_entries(&expected.entries[i], &actual.entries[i]))
            return false;
    }

    return true;
}

static void
run_test_case(TestCase tc, int i, FILE *stream)
{
    Cfg cfg;
    CfgEntry *entries = malloc(MAX_ENTRIES * sizeof(CfgEntry));

    if (entries == NULL) {
        fprintf(stream, "FATAL: malloc() failed");
        exit(1);
    }

    cfg_init(&cfg, entries, MAX_ENTRIES);

    CfgError err;
    int res = cfg_parse(tc.src, strlen(tc.src), &cfg, &err);

    switch (tc.type) {
    case TC_SUCC:
        if (res != 0) {
            fprintf(stream, "SUCCESS CASE %d - " RED "FAILED\n" RESET, i);
            cfg_fprint_error(stream, &err);
        } else {
            if (compare_configs(tc.exp.cfg, cfg)) {
                fprintf(stream, "SUCCESS CASE %d - " GREEN "PASSED\n" RESET, i);
            } else {
                fprintf(stream, "SUCCESS CASE %d - " RED "FAILED\n" RESET, i);
            }
        }
        break;

    case TC_ERR:
        if (res != 0 && !strcmp(tc.exp.err, err.msg)) {
            fprintf(stream, "ERROR CASE %d - " GREEN "PASSED\n" RESET, i);
        } else {
            fprintf(stream, "ERROR CASE %d - " RED "FAILED\n" RESET, i);
            cfg_fprint_error(stream, &err);
        }
        break;

    default:
        fprintf(stream, "FATAL: invalid TestCase type");
        exit(1);
    }

    free(entries);
}

int
main(void)
{
    int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        run_test_case(cases[i], i, stdout);
    }
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

// ---

// gcc -Wall -Wextra -g test.c config.c -o test
