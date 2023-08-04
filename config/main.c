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

    Cfg cfg;
    CfgEntry *entries = malloc(64 * sizeof(CfgEntry));

    if (entries == NULL) {
        fprintf(stderr, "Error: memory allocation failed\n");
        return 1;
    }

    cfg_init(&cfg, entries, 64);

    char err[MAX_ERR_LEN + 1];
    int res = cfg_load(argv[1], &cfg, err);

    if (res != 0) {
        fprintf(stderr, "%s\n", err);
        free(entries);
        return 1;
    }

    cfg_print(cfg);
    printf("%d\n", cfg_get_int(cfg, "fontSize", 30));

    free(entries);
    return 0;
}
