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
    Cfg cfg;
    CfgEntry *entries = malloc(max_entries * sizeof(CfgEntry));

    if (entries == NULL) {
        fprintf(stderr, "Error: memory allocation failed\n");
        return 1;
    }

    cfg_init(&cfg, entries, max_entries);

    char err[MAX_ERR_LEN + 1];
    int res = cfg_load(argv[1], &cfg, err);

    if (res != 0) {
        fprintf(stderr, "%s\n", err);
        free(entries);
        return 1;
    }

    cfg_print(cfg);

    fprintf(stdout, "Testing out the getters:\n");
    fprintf(stdout, "%s\n", cfg_get_str(cfg, "font", "err"));
    fprintf(stdout, "%s\n", cfg_get_bool(cfg, "ruler", false) ? "true" : "false");
    fprintf(stdout, "%d\n", cfg_get_int(cfg, "fontSize", 30));
    fprintf(stdout, "%f\n", cfg_get_float(cfg, "x", 5.5));

    free(entries);
    return 0;
}
