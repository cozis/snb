#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>

typedef enum {
    TEST_PASSED,
    TEST_FAILED,
    TEST_ABORTED,
} TestResultType;

typedef struct {
    TestResultType type;
    const char *file;
    int         line;
} TestResult;

#define ASSERT(X)              \
    if (!(X))                  \
        return (TestResult) {  \
            .type=TEST_FAILED, \
            .file=__FILE__,    \
            .line=__LINE__     \
        }

#define OK (TestResult) {.type=TEST_PASSED}

#define COUNT_OF(X) (sizeof(X) / sizeof((X)[0]))

#define WRAP(E) (Cfg) {        \
        .entries=(E),          \
        .capacity=COUNT_OF(E), \
        .count=COUNT_OF(E)     \
    }

void logResult(TestResult result, FILE *stream);
void updateScoreboard(Scoreboard *scoreboard, TestResult result);

#endif
