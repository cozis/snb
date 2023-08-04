#include <stdint.h>
#include <stdlib.h>

#include "config.h"

int
LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    Cfg cfg;
    CfgEntry *entries = malloc(256 * sizeof(CfgEntry));

    if (entries == NULL)
        return 1;

    cfg_init(&cfg, entries, 64);

    char err[MAX_ERR_LEN + 1];
    int res = cfg_parse(Data, Size, &cfg, err);
    free(entries);
    return 0;
}

// Deps: clang, llvm
// Compile with: clang -O1 -fsanitize=fuzzer,address config_fuzz.c -g
// Run with: ./a.out
