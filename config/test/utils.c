#include "utils.h"

void
log_result(TestResult result, FILE *stream)
{
    char *first;
    char *color;

    switch (result.type) {
    case TEST_PASSED:
        first = "PASSED";
        color = GREEN;
        break;

    case TEST_FAILED:
        first = "FAILED";
        color = RED;
        break;

    case TEST_ABORTED:
        first = "ABORTED";
        color = RED;  // Same color as TEST_FAILED?
        break;
    }

    fprintf(stream, "%s%s" RESET " - %s:%d\n", color, first, result.file,
            result.line);
}

void
update_scoreboard(Scoreboard *sb, TestResult result)
{
    switch (result.type) {
    case TEST_PASSED:
        sb->passed++;
        break;
    case TEST_FAILED:
        sb->failed++;
        break;
    case TEST_ABORTED:
        sb->aborted++;
        break;
    }
}
