#include <string.h>
#include "test.h"
#include "test_load.h"
#include "../config.h"

static TestResult
run_test_case(void)
{
    CfgError err;
    CfgEntry entries[10];
    Cfg cfg = WRAP(entries);
    
    ASSERT(  0 == cfg_load("sample.cfg", &cfg, &err) );
    
    ASSERT( -1 == cfg_load("sample.txt", &cfg, &err) );
    ASSERT( !strcmp("invalid file extension", err.msg));

    ASSERT( -1 == cfg_load("", &cfg, &err) );
    ASSERT( !strcmp("invalid file name", err.msg));

    ASSERT( -1 == cfg_load("sample2.cfg", &cfg, &err) );
    ASSERT( !strcmp("failed to open file", err.msg));

    return OK;
}

void
run_load_tests(Scoreboard *scoreboard, FILE *stream)
{
    TestResult result = run_test_case();
    updateScoreboard(scoreboard, result);
    logResult(result, stream);
}
