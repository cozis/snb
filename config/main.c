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

    FILE *file = fopen(argv[1], "r");

    if (!file) {
        fprintf(stderr, "Error: failed to open the file\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *src = malloc(size);

    if (src == NULL) {
        fprintf(stderr, "Error: buffer allocation failed\n");
        return 1;
    }

    size_t bytes_read = fread(src, sizeof(char), size, file);
    fclose(file);

    if (bytes_read != size) {
        fprintf(stderr, "Error: failed to read the file\n");
        free(src);
        return 1;
    }

    src[size] = '\0';

    char err[MAX_ERR_LEN];
    const int max_entries = 32;
    CfgEntry entries[max_entries];
    int num_entries = parse(src, strlen(src), entries, max_entries, err);

    if (num_entries < 0) {
        fprintf(stderr, "%s\n", err);
        free(src);
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
            free(src);
            return 1;
        }
    }
    free(src);
    return 0;
}
