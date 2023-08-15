#include "utils.h"

void
logResult(TestResult result, FILE *stream)
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
        color = RED; // Same color as TEST_FAILED?
        break;
    }

    fprintf(stream, "%s%s" RESET " - %s:%d\n", 
            color, first, result.file, result.line);
}

void 
updateScoreboard(Scoreboard *scoreboard, TestResult result)
{
    switch (result.type) {
        case TEST_PASSED: scoreboard->passed++; break;
        case TEST_FAILED: scoreboard->failed++; break;
        case TEST_ABORTED: scoreboard->aborted++; break;
    }
}