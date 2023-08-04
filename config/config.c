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

static Scanner scanner;

static void
init_scanner(const char *src, int src_len)
{
    scanner.src = src;
    scanner.len = src_len;
    scanner.cur = 0;
}

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
is_at_end()
{
    return scanner.cur >= scanner.len;
}

static char
advance()
{
    return scanner.src[scanner.cur++];
}

static char
peek()
{
    return scanner.src[scanner.cur];
}

static void
skip_whitespace()
{
    while (!is_at_end() && isspace(peek()))
        scanner.cur++;
}

int
cfg_parse(const char *src,
          int src_len,
          CfgEntry *entries,
          int max_entries,
          char *err)
{
    int i = 0;
    int count = 0;
    init_scanner(src, src_len);

    // Skip leading whitespace
    skip_whitespace();

    while (!is_at_end() && count < max_entries) {
        // Missing key
        if (is_at_end() || !is_key(peek())) {
            char *fmt = "CfgError: missing key in entry %d";
            snprintf(err, MAX_ERR_LEN, fmt, count + 1);
            return -1;
        }

        // Consume key
        i = 0;
        while (!is_at_end() && i < MAX_KEY_LEN && is_key(peek()))
            entries[count].key[i++] = advance();

        entries[count].key[i++] = '\0';

        // Skip whitespace between the key and ':'
        skip_whitespace();

        if (is_at_end() || peek() != ':') {
            char *fmt = "CfgError: ':' expected in entry %d";
            snprintf(err, MAX_ERR_LEN, fmt, count + 1);
            return -1;
        }

        // Consume ':'
        advance();

        // Skip whitespace between ':' and value
        skip_whitespace();

        // Missing value
        if (is_at_end() || peek() == '\n') {
            char *fmt = "CfgError: missing value in entry %d";
            snprintf(err, MAX_ERR_LEN, fmt, count + 1);
            return -1;
        }

        // Consume value
        char c = peek();

        if (isalpha(c) || ispunct(c)) {
            entries[count].type = TYPE_STR;
            i = 0;

            // Copy all the value
            while (!is_at_end() && i < MAX_VAL_LEN && is_value(peek()))
                entries[count].val.str[i++] = advance();
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

            while (!is_at_end() && isdigit(peek()))
                int_part = int_part * 10 + (advance() - '0');

            if (!is_at_end() && peek() == '.') {
                advance();
                is_float = true;
            }

            if (is_float) {
                int div = 1;
                while (!is_at_end() && isdigit(peek())) {
                    fract_part = fract_part * 10 + (advance() - '0');
                    div *= 10;
                }

                entries[count].type = TYPE_FLOAT;
                entries[count].val.float_ = int_part + (fract_part / div);
            } else {
                entries[count].type = TYPE_INT;
                entries[count].val.int_ = int_part;
            }
        } else {
            char *fmt = "CfgError: invalid value in entry %d";
            snprintf(err, MAX_ERR_LEN, fmt, count + 1);
            return -1;
        }

        count++;

        // Skip whitespace for the next entry
        skip_whitespace();
    }

    return count;
}

int
cfg_load(const char *filename, CfgEntry *entries, int max_entries, char *err)
{
    char *ext = strrchr(filename, '.');
    if (strcmp(ext, ".cfg") != 0) {
        strncpy(err, "CfgError: invalid file extension", MAX_ERR_LEN);
        return -1;
    }

    FILE *file = fopen(filename, "r");

    if (!file) {
        strncpy(err, "CfgError: failed to open the file", MAX_ERR_LEN);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *src = malloc(size + 1);

    if (src == NULL) {
        strncpy(err, "CfgError: memory allocation failed", MAX_ERR_LEN);
        return -1;
    }

    size_t bytes_read = fread(src, sizeof(char), size, file);
    fclose(file);

    if (bytes_read != size) {
        strncpy(err, "CfgError: failed to read the file", MAX_ERR_LEN);
        free(src);
        return -1;
    }

    src[size] = '\0';

    int entries_size = cfg_parse(src, strlen(src), entries, max_entries, err);

    free(src);
    return entries_size;
}

int
cfg_get_int(CfgEntry *entries, int entries_size, const char *key, int default_)
{
    for (int i = entries_size - 1; i >= 0; i--) {
        if (entries[i].type == TYPE_INT && !strcmp(key, entries[i].key))
            return entries[i].val.int_;
    }

    return default_;
}

float
cfg_get_float(CfgEntry *entries,
              int entries_size,
              const char *key,
              float default_)
{
    for (int i = entries_size - 1; i >= 0; i--) {
        if (entries[i].type == TYPE_FLOAT && !strcmp(key, entries[i].key)) {
            return entries[i].val.float_;
        }
    }

    return default_;
}

char *
cfg_get_str(CfgEntry *entries,
            int entries_size,
            const char *key,
            char *default_)
{
    for (int i = entries_size - 1; i >= 0; i--) {
        if (entries[i].type == TYPE_STR && !strcmp(key, entries[i].key))
            return entries[i].val.str;
    }

    return default_;
}
