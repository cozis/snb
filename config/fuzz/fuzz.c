#include <stdint.h>
#include <stdlib.h>

#include "../config.h"

int
LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    int max_entries = 64;
    Cfg cfg;
    CfgEntry *entries = malloc(max_entries * sizeof(CfgEntry));

    if (entries == NULL)
        return 1;

    cfg_init(&cfg, entries, max_entries);

    CfgError err;
    int res = cfg_parse(Data, Size, &cfg, &err);

    free(entries);
    return 0;
}

// Run: ./fzz
//      ./fzz ./fuzz/corpus/
