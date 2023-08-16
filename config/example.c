#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Error: missing config file\n");
        return 1;
    }

    int max_entries = 64;
    ConfigEntry entries[max_entries];
    ConfigError err;

    int count = Config_parseFile(argv[1], entries, max_entries, &err);

    if (count == -1) {
        config_fprint_error(stderr, &err);
        return 1;
    }

    ConfigEntry *font = Config_getEntry(entries, count, "font");
    fprintf(stdout, "Font: %s\n", font->val.string);

    return 0;
}
