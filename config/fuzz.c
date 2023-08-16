#include <stdint.h>
#include <stdlib.h>

#include "config.h"

int
LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    int max_entries = 64;
    ConfigEntry entries[max_entries];
    ConfigError err;

    int count = Config_parse(Data, Size, entries, max_entries, &err);

    return 0;
}

// Docs: https://llvm.org/docs/LibFuzzer.html
