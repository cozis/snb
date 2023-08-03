#include <stdint.h>
#include <stdlib.h>

#include "config.h"

int
LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    CfgEntry entries[64];
    char err[64 + 1];
    cfg_parse(Data, Size, entries, 64, err, 64);
    return 0;
}

// Deps: clang, llvm
// Compile with: clang -O1 -fsanitize=fuzzer,address config_fuzz.c -g
// Run with: ./a.out
