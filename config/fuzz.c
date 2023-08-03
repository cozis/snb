#include <stdint.h>
#include <stdlib.h>

#include "config.h"

int
LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    int max_entries = 64;
    CfgEntry entries[max_entries];
    char err[MAX_ERR_LEN + 1];
    parse(Data, Size, entries, max_entries, err);
    return 0;
}

// Deps: clang, llvm
// Compile with: clang -O1 -fsanitize=fuzzer,address config_fuzz.c -g
// Run with: ./a.out
