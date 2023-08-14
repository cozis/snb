#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../config.h"
#include "test.h"
#include "test_print.h"

static FILE *stream;

void
run_print_err_test_1(Scoreboard *scoreboard)
{
    Cfg cfg;
    CfgEntry entries[TEST_CAPACITY];

    cfg_init(&cfg, entries, TEST_CAPACITY);

    CfgError err;
    int res = cfg_load("", &cfg, &err);

    char expected[] = "Error: invalid filename\n";

    char buffer[256];
    FILE *tmp = fmemopen(buffer, sizeof(buffer), "w");
    if (tmp == NULL) {
        fprintf(stream, "FATAL: fmemopen() failed");
        return;
    }

    cfg_fprint_error(tmp, &err);
    fclose(tmp);

    scoreboard->total++;

    if (strncmp(buffer, expected, strlen(expected)) != 0) {
        cfg_fprint_error(stream, &err);
        fprintf(stream, "SUCCESS CASE - " RED "FAILED " RESET "(%s:%d)\n", __FILE__,
                __LINE__);
        scoreboard->failed++;
    } else {
        fprintf(stream, "SUCCESS CASE - " GREEN "PASSED " RESET "(%s:%d)\n",
                __FILE__, __LINE__);
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
    int res = cfg_parse(src, strlen(src), &cfg, &err);

    char expected[] = "Error at 2:3 :: missing value\n";

    char buffer[256];
    FILE *tmp = fmemopen(buffer, sizeof(buffer), "w");
    if (tmp == NULL) {
        fprintf(stream, "FATAL: fmemopen() failed");
        return;
    }

    cfg_fprint_error(tmp, &err);
    fclose(tmp);

    scoreboard->total++;

    if (strncmp(buffer, expected, strlen(expected)) != 0) {
        cfg_fprint_error(stream, &err);
        fprintf(stream, "SUCCESS CASE - " RED "FAILED " RESET "(%s:%d)\n", __FILE__,
                __LINE__);
        scoreboard->failed++;
    } else {
        fprintf(stream, "SUCCESS CASE - " GREEN "PASSED " RESET "(%s:%d)\n",
                __FILE__, __LINE__);
        scoreboard->passed++;
    }
}

void
run_print_test(Scoreboard *scoreboard)
{
    Cfg cfg = {
        .entries =
            (CfgEntry[]){
                {.key = "font",
                 .type = CFG_TYPE_STR,
                 .val.str = "JetBrainsMono Nerd Font"},
                {.key = "font.size", .type = CFG_TYPE_INT, .val.int_ = 14},
                {.key = "zoom", .type = CFG_TYPE_FLOAT, .val.float_ = 1.5},
                {.key = "line_numbers", .type = CFG_TYPE_BOOL, .val.bool_ = true},
                {.key = "ruler", .type = CFG_TYPE_BOOL, .val.bool_ = false},
                {.key = "bg.color",
                 .type = CFG_TYPE_COLOR,
                 .val.color = {.r = 255, .g = 255, .b = 255, .a = 255}},
            },
        .capacity = TEST_CAPACITY,
        .count = 6,
    };

    char expected[] = "font: \"JetBrainsMono Nerd Font\"\n"
                      "font.size: 14\n"
                      "zoom: 1.500000\n"
                      "line_numbers: true\n"
                      "ruler: false\n"
                      "bg.color: rgba(255, 255, 255, 255)";

    char buffer[256];
    FILE *tmp = fmemopen(buffer, sizeof(buffer), "w");
    if (tmp == NULL) {
        fprintf(stream, "FATAL: fmemopen() failed");
        return;
    }

    cfg_fprint(tmp, cfg);
    fclose(tmp);

    scoreboard->total++;

    if (strncmp(buffer, expected, strlen(expected)) != 0) {
        fprintf(stream, "SUCCESS CASE - " RED "FAILED " RESET "(%s:%d)\n", __FILE__,
                __LINE__);
        scoreboard->failed++;
    } else {
        fprintf(stream, "SUCCESS CASE - " GREEN "PASSED " RESET "(%s:%d)\n",
                __FILE__, __LINE__);
        scoreboard->passed++;
    }
}

void
run_print_tests(Scoreboard *scoreboard, FILE *stream_)
{
    stream = stream_;
    run_print_test(scoreboard);
    run_print_err_test_1(scoreboard);
    run_print_err_test_2(scoreboard);
}
