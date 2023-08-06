#include <ctype.h>
#include <stdarg.h>
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

static bool
is_key(char ch)
{
    return isalpha(ch) || ch == '.';
}

static bool
is_value(char ch)
{
    return isalnum(ch) || isblank(ch) || (ispunct(ch) && ch != '"');
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
advance2(int n)
{
    for (int i = 0; i < n - 1; i++)
        advance();
    return advance();
}

static char
peek()
{
    return scanner.src[scanner.cur];
}

static char
peek_next()
{
    if (scanner.cur >= scanner.len - 1)
        return '\0';
    return scanner.src[scanner.cur + 1];
}

static int
cur()
{
    return scanner.cur;
}

static void
skip_whitespace()
{
    while (!is_at_end() && isspace(peek()))
        advance();
}

static void
skip_blank()
{
    while (!is_at_end() && isblank(peek()))
        advance();
}

void
skip_comment()
{
    while (!is_at_end() && peek() == '#') {
        do
            advance();
        while (!is_at_end() && peek() != '\n');
    }
}

void
skip_whitespace_and_comments()
{
    while (!is_at_end() && (isspace(peek()) || peek() == '#')) {
        skip_whitespace();
        skip_comment();
    }
}

static bool
check_literal(int offset, const char *literal, int len)
{
    if (offset + len > scanner.len)
        return false;
    return !strncmp(scanner.src + offset, literal, len);
}

static int
error(const char *fmt, char *err, ...)
{
    const char prefix[] = "CfgError: ";
    const int prefix_len = sizeof(prefix) - 1;

    va_list vargs;
    va_start(vargs, err);
    snprintf(err, MAX_ERR_LEN + 1, prefix);
    vsnprintf(err + prefix_len, MAX_ERR_LEN - prefix_len + 1, fmt, vargs);
    va_end(vargs);
    return -1;
}

void
cfg_init(Cfg *cfg, CfgEntry *entries, int max_entries)
{
    cfg->entries = entries;
    cfg->max_entries = max_entries;
    cfg->size = 0;
}

int
cfg_parse(const char *src, int src_len, Cfg *cfg, char *err)
{
    int count = 0;
    init_scanner(src, src_len);

    // Skip initial whitespace and comments
    skip_whitespace_and_comments();

    while (!is_at_end() && count < cfg->max_entries) {
        // Missing key
        if (is_at_end() || !is_key(peek()))
            return error("missing key in entry %d", err, count + 1);

        // Consume key
        int key_offset = cur();
        do
            advance();
        while (!is_at_end() && is_key(peek()));
        int key_len = cur() - key_offset;

        if (key_len > MAX_KEY_LEN)
            return error("key too long in entry %d", err, count + 1);

        memcpy(cfg->entries[count].key, src + key_offset, key_len);
        cfg->entries[count].key[key_len] = '\0';

        // Skip blank space between the key and ':'
        skip_blank();

        if (is_at_end() || peek() != ':')
            return error("':' expected in entry %d", err, count + 1);

        // Consume ':'
        advance();

        // Skip blank space between ':' and value
        skip_blank();

        // Missing value
        if (is_at_end() || peek() == '\n')
            return error("missing value in entry %d", err, count + 1);

        // Consume value
        char c = peek();

        if (c == '"') {
            // Consume opening '"'
            advance();

            int val_offset = cur();
            while (!is_at_end() && is_value(peek()))
                advance();

            if (is_at_end() || peek() != '"')
                return error("closing '\"' expected in entry %d", err, count + 1);

            int val_len = cur() - val_offset;
            if (val_len > MAX_VAL_LEN)
                return error("value too long in entry %d", err, count + 1);

            // Consume closing '"'
            advance();

            memcpy(cfg->entries[count].val.str, src + val_offset, val_len);
            cfg->entries[count].val.str[val_len] = '\0';
            cfg->entries[count].type = TYPE_STR;
        } else if (isalpha(c)) {
            bool bool_;
            switch (c) {
            case 't':
                if (!check_literal(cur(), "true", 4))
                    return error("invalid literal in entry %d", err, count + 1);
                // Consume "true"
                advance2(4);
                bool_ = true;
                break;
            case 'f':
                if (!check_literal(cur(), "false", 5))
                    return error("invalid literal in entry %d", err, count + 1);
                // Consume "false"
                advance2(5);
                bool_ = false;
                break;
            default:
                return error("invalid literal in entry %d", err, count + 1);
            }
            cfg->entries[count].val.bool_ = bool_;
            cfg->entries[count].type = TYPE_BOOL;
        } else if (isdigit(c) || (c == '-' && isdigit(peek_next()))) {
            bool is_neg = false;
            bool is_float = false;
            int int_part = 0;
            float fract_part = 0;

            if (c == '-' && isdigit(peek_next())) {
                advance();
                is_neg = true;
            }

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
                float float_ = int_part + (fract_part / div);
                cfg->entries[count].val.float_ = is_neg ? -1 * float_ : float_;
                cfg->entries[count].type = TYPE_FLOAT;
            } else {
                cfg->entries[count].val.int_ = is_neg ? -1 * int_part : int_part;
                cfg->entries[count].type = TYPE_INT;
            }
        } else {
            return error("invalid value in entry %d", err, count + 1);
        }

        count++;

        // Skip whitespace and comments for the next entry
        skip_whitespace_and_comments();
    }

    cfg->size = count;
    return 0;
}

int
cfg_load(const char *filename, Cfg *cfg, char *err)
{
    char *ext = strrchr(filename, '.');
    if (strcmp(ext, ".cfg") != 0)
        return error("invalid file extension", err);

    FILE *file = fopen(filename, "rb");
    if (!file)
        return error("failed to open the file", err);

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *src = malloc(size + 1);
    if (src == NULL)
        return error("memory allocation failed", err);

    size_t bytes_read = fread(src, sizeof(char), size, file);
    fclose(file);

    if (bytes_read != size) {
        free(src);
        return error("failed to read the file", err);
    }

    src[size] = '\0';
    int res = cfg_parse(src, strlen(src), cfg, err);

    free(src);
    return res;
}

static void *
get_val(Cfg cfg, const char *key, void *default_, CfgValType type)
{
    for (int i = cfg.size - 1; i >= 0; i--) {
        if (cfg.entries[i].type == type && !strcmp(key, cfg.entries[i].key))
            return &cfg.entries[i].val;
    }
    return default_;
}

char *
cfg_get_str(Cfg cfg, const char *key, char *default_)
{
    return (char *) get_val(cfg, key, default_, TYPE_STR);
}

bool
cfg_get_bool(Cfg cfg, const char *key, bool default_)
{
    return *(bool *) get_val(cfg, key, &default_, TYPE_BOOL);
}

int
cfg_get_int(Cfg cfg, const char *key, int default_)
{
    return *(int *) get_val(cfg, key, &default_, TYPE_INT);
}

float
cfg_get_float(Cfg cfg, const char *key, float default_)
{
    return *(float *) get_val(cfg, key, &default_, TYPE_FLOAT);
}

void
cfg_print(Cfg cfg)
{
    for (int i = 0; i < cfg.size; i++) {
        fprintf(stdout, "%s: ", cfg.entries[i].key);

        switch (cfg.entries[i].type) {
        case TYPE_STR:
            fprintf(stdout, "\"%s\"\n", cfg.entries[i].val.str);
            break;
        case TYPE_BOOL:
            fprintf(stdout, "%s\n", cfg.entries[i].val.bool_ ? "true" : "false");
            break;
        case TYPE_INT:
            fprintf(stdout, "%d\n", cfg.entries[i].val.int_);
            break;
        case TYPE_FLOAT:
            fprintf(stdout, "%f\n", cfg.entries[i].val.float_);
            break;
        default:
            fprintf(stderr, "CfgError: unknown type\n");
        }
    }
}
