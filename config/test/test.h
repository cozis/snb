#ifndef TEST_H
#define TEST_H

// #define LOGFILE

#ifdef LOGFILE
#define RED
#define GREEN
#define RESET
#else
#define RED   "\e[1;31m"
#define GREEN "\e[1;32m"
#define RESET "\e[0m"
#endif

#define TEST_CAPACITY 64

typedef struct {
    int aborted;
    int passed;
    int failed;
} Scoreboard;

#endif