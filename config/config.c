#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

typedef struct {
    const char *src;
    int len;
    int cur;
} Scanner;

static void
rm_dup_whitespace(char *str)
{
    int j = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isblank(str[i]) || (i > 0 && !isblank(str[i - 1])))
            str[j++] = str[i];
    }
    str[j] = '\0';
}

static bool
is_key(char ch)
{
    return isalpha(ch) || ch == '.';
}

static bool
is_value(char ch)
{
    return isalnum(ch) || isblank(ch) || ispunct(ch);
}

static bool
is_at_end(Scanner *scanner)
{
    return scanner->cur >= scanner->len;
}

static char
advance(Scanner *scanner)
{
    return scanner->src[scanner->cur++];
}

static char
peek(Scanner *scanner)
{
    return scanner->src[scanner->cur];
}

static void
skip_whitespace(Scanner *scanner)
{
    while (!is_at_end(scanner) && isspace(peek(scanner)))
        scanner->cur++;
}

int
cfg_parse(const char *src,
          int src_len,
          CfgEntry *entries,
          int max_entries,
          char *err,
          int err_len)
{
    Scanner scanner = {.src = src, .len = src_len, .cur = 0};
    int i = 0;
    int count = 0;

    // Skip leading whitespace
    skip_whitespace(&scanner);

    while (!is_at_end(&scanner) && count < max_entries) {
        // Missing key
        if (is_at_end(&scanner) || !is_key(peek(&scanner))) {
            char *fmt = "Error: missing key in entry %d";
            snprintf(err, err_len + 1, fmt, count + 1);
            return -1;
        }

        // Consume key
        i = 0;
        while (!is_at_end(&scanner) && i < MAX_KEY_LEN &&
               is_key(peek(&scanner)))
            entries[count].key[i++] = advance(&scanner);

        entries[count].key[i++] = '\0';

        // Skip whitespace between the key and ':'
        skip_whitespace(&scanner);

        if (is_at_end(&scanner) || peek(&scanner) != ':') {
            char *fmt = "Error: ':' expected in entry %d";
            snprintf(err, err_len + 1, fmt, count + 1);
            return -1;
        }

        // Consume ':'
        advance(&scanner);

        // Skip whitespace between ':' and value
        skip_whitespace(&scanner);

        // Missing value
        if (is_at_end(&scanner) || peek(&scanner) == '\n') {
            char *fmt = "Error: missing value in entry %d";
            snprintf(err, err_len + 1, fmt, count + 1);
            return -1;
        }

        // Consume value
        char c = peek(&scanner);

        if (isalpha(c) || ispunct(c)) {
            entries[count].type = TYPE_STR;
            i = 0;

            // Copy all the value
            while (!is_at_end(&scanner) && i < MAX_VAL_LEN &&
                   is_value(peek(&scanner)))
                entries[count].val.str[i++] = advance(&scanner);
            i--;

            // Remove trailing whitespace
            while (i > 0 && isblank(entries[count].val.str[i]))
                i--;

            entries[count].val.str[++i] = '\0';
            rm_dup_whitespace(entries[count].val.str);
        } else if (isdigit(c)) {
            bool is_float = false;
            int int_part = 0;
            float fract_part = 0;

            while (!is_at_end(&scanner) && isdigit(peek(&scanner)))
                int_part = int_part * 10 + (advance(&scanner) - '0');

            if (!is_at_end(&scanner) && peek(&scanner) == '.') {
                advance(&scanner);
                is_float = true;
            }

            if (is_float) {
                int div = 1;
                while (!is_at_end(&scanner) && isdigit(peek(&scanner))) {
                    fract_part = fract_part * 10 + (advance(&scanner) - '0');
                    div *= 10;
                }

                entries[count].type = TYPE_FLOAT;
                entries[count].val.float_ = int_part + (fract_part / div);
            } else {
                entries[count].type = TYPE_INT;
                entries[count].val.int_ = int_part;
            }
        } else {
            char *fmt = "Error: invalid value in entry %d";
            snprintf(err, err_len + 1, fmt, count + 1);
            return -1;
        }

        count++;

        // Skip whitespace for the next entry
        skip_whitespace(&scanner);
    }

    return count;
}

int
cfg_load(const char *filename,
         CfgEntry *entries,
         int max_entries,
         char *err,
         int err_len)
{
    char *ext = strrchr(filename, '.');
    if (strcmp(ext, ".cfg") != 0) {
        strncpy(err, "Error: invalid file extension", err_len);
        return -1;
    }

    FILE *file = fopen(filename, "r");

    if (!file) {
        strncpy(err, "Error: failed to open the file", err_len);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *src = malloc(size);

    if (src == NULL) {
        strncpy(err, "Error: buffer allocation failed", err_len);
        return -1;
    }

    size_t bytes_read = fread(src, sizeof(char), size, file);
    fclose(file);

    if (bytes_read != size) {
        strncpy(err, "Error: failed to read the file", err_len);
        free(src);
        return -1;
    }

    src[size] = '\0';

    int num_entries =
        cfg_parse(src, strlen(src), entries, max_entries, err, err_len);

    free(src);
    return num_entries;
}

CfgEntry *
cfg_get(const char *key, CfgEntry *entries, int num_entries)
{
    for (int i = num_entries - 1; i >= 0; i--) {
        if (!strcmp(key, entries[i].key))
            return &entries[i];
    }

    return NULL;
}
