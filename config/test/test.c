#include <stdio.h>

#include "test.h"
#include "test_get.h"
#include "test_load.h"
#include "test_parse.h"
#include "test_print.h"

int
main(void)
{
#ifdef LOGFILE
    FILE *stream = fopen("log.txt", "w");
    if (!stream) {
        fprintf(stderr, "FATAL: Failed to open log file\n");
        return 1;
    }
#else
    FILE *stream = stdout;
#endif

    Scoreboard scoreboard = {0};
    run_parse_tests(&scoreboard, stream);
    run_load_tests(&scoreboard, stream);
    run_get_tests(&scoreboard, stream);
    run_print_test(&scoreboard, stream);

    fprintf(stream, "Total: %d Passed: %d Failed: %d\n", scoreboard.total,
            scoreboard.passed, scoreboard.failed);

#ifdef LOGFILE
    fclose(stream);
#endif
    return 0;
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
