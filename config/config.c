#include <assert.h>
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

static void
init_scanner(Scanner *s, const char *src, int src_len)
{
    s->src = src;
    s->len = src_len;
    s->cur = 0;
}

static bool
is_at_end(Scanner *s)
{
    return s->cur >= s->len;
}

static int
cur(Scanner *s)
{
    return s->cur;
}

static char
peek(Scanner *s)
{
    return s->src[s->cur];
}

static char
peek_next(Scanner *s)
{
    if (s->cur >= s->len - 1)
        return '\0';
    return s->src[s->cur + 1];
}

static char
advance(Scanner *s)
{
    return s->src[s->cur++];
}

static char
advance2(Scanner *s, int n)
{
    for (int i = 0; i < n - 1; i++)
        advance(s);
    return advance(s);
}

static void
skip_whitespace(Scanner *s)
{
    while (!is_at_end(s) && isspace(peek(s)))
        advance(s);
}

static void
skip_blank(Scanner *s)
{
    while (!is_at_end(s) && isspace(peek(s)) && peek(s) != '\n')
        advance(s);
}

static void
skip_comment(Scanner *s)
{
    while (!is_at_end(s) && peek(s) == '#') {
        do
            advance(s);
        while (!is_at_end(s) && peek(s) != '\n');
    }
}

void
skip_whitespace_and_comments(Scanner *s)
{
    while (!is_at_end(s) && (isspace(peek(s)) || peek(s) == '#')) {
        skip_whitespace(s);
        skip_comment(s);
    }
}

static void
copy_slice_into(Scanner *s, int src_off, int src_len, char *dst, int max)
{
    assert(src_len < max);
    memcpy(dst, s->src + src_off, src_len);
    dst[src_len] = '\0';
}

static bool
match_literal(Scanner *s, int offset, const char *literal, int len)
{
    if (offset + len > s->len)
        return false;
    return !strncmp(s->src + offset, literal, len);
}

static bool
is_key(char ch)
{
    return isalpha(ch) || ch == '.' || ch == '_';
}

static bool
is_string(char ch)
{
    return isalnum(ch) || isblank(ch) || (ispunct(ch) && ch != '"');
}

static void
init_error(ConfigError *err)
{
    err->off = -1;
    err->col = -1;
    err->row = -1;
    err->msg[0] = '\0';
}

static int
error(Scanner *s, ConfigError *err, const char *fmt, ...)
{
    const char prefix[] = "";
    const int prefix_len = sizeof(prefix) - 1;

    err->off = cur(s);
    err->row = 1;
    err->col = 1;
    for (int i = 0; i < cur(s); i++) {
        err->col++;
        if (s->src[i] == '\n') {
            err->row++;
            err->col = 1;
        }
    }

    va_list vargs;
    va_start(vargs, fmt);
    snprintf(err->msg, CONFIG_MAX_ERR, prefix);
    vsnprintf(err->msg + prefix_len, CONFIG_MAX_ERR - prefix_len, fmt, vargs);
    va_end(vargs);
    return -1;
}

void
config_fprint_error(FILE *stream, ConfigError *err)
{
    if (err->row == -1 && err->col == -1)
        fprintf(stream, "Error: %s\n", err->msg);
    else
        fprintf(stream, "Error at %d:%d :: %s\n", err->row, err->col, err->msg);
}

static int
parse_string(Scanner *s, ConfigEntry *entry, ConfigError *err)
{
    // Consume opening '"'
    advance(s);

    // Consume string
    int val_offset = cur(s);
    while (!is_at_end(s) && is_string(peek(s)))
        advance(s);

    if (is_at_end(s) || peek(s) != '"')
        return error(s, err, "closing '\"' expected");

    int val_len = cur(s) - val_offset;
    if (val_len > CONFIG_MAX_VAL)
        return error(s, err, "value too long");

    // Consume closing '"'
    advance(s);

    copy_slice_into(s, val_offset, val_len, entry->val.string,
                    sizeof(entry->val.string));
    entry->type = CONFIG_TYPE_STRING;
    return 0;
}

static int
consume_number(Scanner *s, float *number, bool *is_int)
{
    bool is_float = false;
    int sign = 1;
    int int_part = 0;
    float fract_part = 0;

    if (!is_at_end(s) && peek(s) == '-' && isdigit(peek_next(s))) {
        // Consume '-'
        advance(s);
        sign = -1;
    }

    if (!is_at_end(s) && !isdigit(peek(s)))
        return -1;

    while (!is_at_end(s) && isdigit(peek(s)))
        int_part = int_part * 10 + (advance(s) - '0');

    if (!is_at_end(s) && peek(s) == '.') {
        advance(s);
        is_float = true;
    }

    if (!is_float) {
        *is_int = true;
        *number = sign * int_part;
    } else {
        int div = 1;
        while (!is_at_end(s) && isdigit(peek(s))) {
            fract_part = fract_part * 10 + (advance(s) - '0');
            div *= 10;
        }
        float floating = int_part + (fract_part / div);
        *is_int = false;
        *number = sign * floating;
    }

    return 0;
}

static int
parse_number(Scanner *s, ConfigEntry *entry, ConfigError *err)
{
    bool is_int;
    float number;
    if (consume_number(s, &number, &is_int) != 0)
        return error(s, err, "number expected");

    if (is_int) {
        entry->val.integer = (int) number;
        entry->type = CONFIG_TYPE_INT;
    } else {
        entry->val.floating = number;
        entry->type = CONFIG_TYPE_FLOAT;
    }

    return 0;
}

static int
parse_rgba(Scanner *s, ConfigEntry *entry, ConfigError *err)
{
    if (!match_literal(s, cur(s), "rgba", 4))
        return error(s, err, "invalid literal");

    // Consume "rgba"
    advance2(s, 4);

    // Skip blank space between 'a' and '('
    skip_blank(s);

    if (is_at_end(s) || peek(s) != '(')
        return error(s, err, "'(' expected");

    // Consume '('
    advance(s);

    bool is_int;
    uint8_t rgb[3];
    for (int i = 0; i < 3; i++) {
        // Skip blank space preceding the number
        skip_blank(s);

        float number;
        if (consume_number(s, &number, &is_int) != 0)
            return error(s, err, "number expected");

        if (!is_int || number < 0 || number > 255)
            return error(s, err,
                         "red, blue and green must be "
                         "integers in range (0, 255)");

        rgb[i] = (uint8_t) number;

        // Skip blank space following the number
        skip_blank(s);

        if (is_at_end(s) || peek(s) != ',')
            return error(s, err, "',' expected");

        // Consume ','
        advance(s);
    }

    // Skip blank space preceding the number
    skip_blank(s);

    float alpha;
    if (consume_number(s, &alpha, &is_int) != 0)
        return error(s, err, "number expected");

    if (alpha < 0 || alpha > 1)
        return error(s, err, "alpha must be in range (0, 1)");

    // Skip blank space following the number
    skip_blank(s);

    if (is_at_end(s) || peek(s) != ')')
        return error(s, err, "')' expected");

    // Consume ')'
    advance(s);

    ConfigColor color = {
        .r = rgb[0],
        .g = rgb[1],
        .b = rgb[2],
        .a = (uint8_t) (alpha * 255),
    };
    entry->val.color = color;
    entry->type = CONFIG_TYPE_COLOR;
    return 0;
}

static int
parse_true(Scanner *s, ConfigEntry *entry, ConfigError *err)
{
    if (!match_literal(s, cur(s), "true", 4))
        return error(s, err, "invalid literal");

    // Consume "true"
    advance2(s, 4);

    entry->val.boolean = true;
    entry->type = CONFIG_TYPE_BOOL;
    return 0;
}

static int
parse_false(Scanner *s, ConfigEntry *entry, ConfigError *err)
{
    if (!match_literal(s, cur(s), "false", 5))
        return error(s, err, "invalid literal");

    // Consume "false"
    advance2(s, 5);

    entry->val.boolean = false;
    entry->type = CONFIG_TYPE_BOOL;
    return 0;
}

static int
parse_literal(Scanner *s, ConfigEntry *entry, ConfigError *err)
{
    switch (peek(s)) {
    case 't':
        return parse_true(s, entry, err);
    case 'f':
        return parse_false(s, entry, err);
    case 'r':
        return parse_rgba(s, entry, err);
    default:
        return error(s, err, "invalid literal");
    }
}

static int
parse_value(Scanner *s, ConfigEntry *entry, ConfigError *err)
{
    // Skip blank space between ':' and the value
    skip_blank(s);

    if (is_at_end(s) || peek(s) == '\n')
        return error(s, err, "missing value");

    // Consume value
    char c = peek(s);

    if (c == '"')
        return parse_string(s, entry, err);
    else if (isalpha(c))
        return parse_literal(s, entry, err);
    else if (isdigit(c) || (c == '-' && isdigit(peek_next(s))))
        return parse_number(s, entry, err);
    else
        return error(s, err, "invalid value");
}

static int
parse_key(Scanner *s, ConfigEntry *entry, ConfigError *err)
{
    if (is_at_end(s) || !is_key(peek(s)))
        return error(s, err, "missing key");

    // Consume key
    int key_offset = cur(s);
    do
        advance(s);
    while (!is_at_end(s) && is_key(peek(s)));
    int key_len = cur(s) - key_offset;

    if (key_len > CONFIG_MAX_KEY)
        return error(s, err, "key too long");

    copy_slice_into(s, key_offset, key_len, entry->key, sizeof(entry->key));
    return 0;
}

static int
consume_colon(Scanner *s, ConfigError *err)
{
    // Skip blank space between the key and ':'
    skip_blank(s);

    if (is_at_end(s) || peek(s) != ':')
        return error(s, err, "':' expected");

    // Consume ':'
    advance(s);
    return 0;
}

static int
parse_entry(Scanner *s, ConfigEntry *entry, ConfigError *err)
{
    if (parse_key(s, entry, err) != 0)
        return -1;

    if (consume_colon(s, err) != 0)
        return -1;

    if (parse_value(s, entry, err) != 0)
        return -1;

    // Skip trailing blank space after the value
    skip_blank(s);

    if (!is_at_end(s) && peek(s) == '#')
        skip_comment(s);

    if (!is_at_end(s) && peek(s) != '\n')
        return error(s, err, "unexpected character '%c'", peek(s));

    // Consume '\n'
    if (!is_at_end(s))
        advance(s);

    return 0;
}

int
Config_parse(const char *src,
             int src_len,
             ConfigEntry *entries,
             int max_entries,
             ConfigError *err)
{
    Scanner s;
    init_scanner(&s, src, src_len);
    init_error(err);

    int count = 0;
    skip_whitespace_and_comments(&s);

    while (!is_at_end(&s) && count < max_entries) {
        ConfigEntry *entry = &entries[count];

        if (parse_entry(&s, entry, err) != 0)
            return -1;

        count++;
        skip_whitespace_and_comments(&s);
    }

    return count;
}

static char *
read_file(const char *filename, int *count, char *err)
{
    FILE *file = fopen(filename, "rb");
    if (!file) {
        snprintf(err, CONFIG_MAX_ERR, "failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = (size_t) ftell(file);
    rewind(file);

    char *src = malloc(file_size + 1);
    if (src == NULL) {
        snprintf(err, CONFIG_MAX_ERR, "memory allocation failed");
        return NULL;
    }

    size_t bytes_read = fread(src, sizeof(char), file_size, file);
    fclose(file);

    if (bytes_read != file_size) {
        free(src);
        snprintf(err, CONFIG_MAX_ERR, "failed to read file");
        return NULL;
    }

    *count = file_size;
    return src;
}

int
Config_parseFile(const char *filename,
                 ConfigEntry *entries,
                 int max_entries,
                 ConfigError *err)
{
    init_error(err);

    size_t len = strlen(filename);
    if (len < 5) {
        snprintf(err->msg, CONFIG_MAX_ERR, "invalid filename");
        return -1;
    }

    const char *ext = filename + (len - strlen(CONFIG_FILE_EXT));
    if (strcmp(ext, CONFIG_FILE_EXT) != 0) {
        snprintf(err->msg, CONFIG_MAX_ERR, "invalid file extension");
        return -1;
    }

    int src_len;
    char *src = read_file(filename, &src_len, err->msg);
    if (src == NULL)
        return -1;

    int res = Config_parse(src, src_len, entries, max_entries, err);

    free(src);
    return res;
}

ConfigEntry *
Config_getEntry(ConfigEntry *entries, int count, const char *key)
{
    for (int i = 0; i < count; i++) {
        if (!strcmp(key, entries[i].key))
            return &entries[i];
    }

    return NULL;
}
