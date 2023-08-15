#include "../config.h"
#include "test.h"
#include "test_get.h"

static TestResult 
run_test_int(void)
{
    CfgEntry entries[] = {
        { .key="keyA", .type=CFG_TYPE_INT, .val=10  },
        { .key="keyB", .type=CFG_TYPE_INT, .val=124 },
    };
    Cfg cfg = WRAP(entries);
    
    ASSERT(  10 == cfg_get_int(cfg, "keyA", 976) );
    ASSERT( 124 == cfg_get_int(cfg, "keyB", 976) );
    ASSERT( 976 == cfg_get_int(cfg, "keyC", 976) );
        
    ASSERT(  10 == cfg_get_int_min(cfg, "keyA", 976,  5) );
    ASSERT(  10 == cfg_get_int_min(cfg, "keyA", 976,  9) );
    ASSERT(  10 == cfg_get_int_min(cfg, "keyA", 976, 10) );
    ASSERT( 976 == cfg_get_int_min(cfg, "keyA", 976, 11) );
    ASSERT( 976 == cfg_get_int_min(cfg, "keyA", 976, 15) );

    ASSERT( 976 == cfg_get_int_max(cfg, "keyA", 976,  5) );
    ASSERT( 976 == cfg_get_int_max(cfg, "keyA", 976,  9) );
    ASSERT(  10 == cfg_get_int_max(cfg, "keyA", 976, 10) );
    ASSERT(  10 == cfg_get_int_max(cfg, "keyA", 976, 11) );
    ASSERT(  10 == cfg_get_int_max(cfg, "keyA", 976, 15) );

    return OK;
}

void 
run_get_tests(Scoreboard *scoreboard, FILE *stream)
{
    TestResult result = run_test_int();
    updateScoreboard(scoreboard, result);
    logResult(result, stream);
}