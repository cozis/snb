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

    CfgEntry entries[64];
    char err[64 + 1];
    int num_entries = cfg_load(argv[1], entries, 64, err, 64);

    if (num_entries < 0) {
        fprintf(stderr, "%s\n", err);
        return 1;
    }

    for (int i = 0; i < num_entries; i++) {
        fprintf(stdout, "%s: ", entries[i].key);

        switch (entries[i].type) {
        case TYPE_STR:
            fprintf(stdout, "%s\n", entries[i].val.str);
            break;
        case TYPE_INT:
            fprintf(stdout, "%d\n", entries[i].val.int_);
            break;
        case TYPE_FLOAT:
            fprintf(stdout, "%f\n", entries[i].val.float_);
            break;
        default:
            fprintf(stderr, "Error: unknown type\n");
            return 1;
        }
    }

    CfgEntry *font = cfg_get("font", entries, num_entries);
    if (font != NULL)
        printf("Key: %s - Val: %s\n", font->key, font->val.str);
    else
        printf("NULL\n");

    return 0;
}
