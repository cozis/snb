#include "config.h"
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
is_string(char ch)
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
    while (!is_at_end() && isspace(peek()) && peek() != '\n')
        advance();
}

static void
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

// FIXME (match?)
static bool
follows_string(int offset, const char *literal, int len)
{
    if (offset + len > scanner.len)
        return false;
    return !strncmp(scanner.src + offset, literal, len);
}

// FIXME
static float
consume_number(bool *is_int)
{
    bool is_float = false;
    int sign = 1;
    int int_part = 0;
    float fract_part = 0;

    if (!is_at_end() && peek() == '-' && isdigit(peek_next())) {
        advance();
        sign = -1;
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
        is_int = false;
        return sign * float_;
    }

    *is_int = true;
    return sign * int_part;
}

void
cfg_fprint_error(FILE *stream, CfgError *err)
{
    fprintf(stream, "Error at %d:%d :: %s\n", err->row, err->col, err->msg);
#ifdef CFG_DETAILED_ERRORS
    fprintf(stream, "\n");
    if (err->row > 0)
        fprintf(stream, "%4d | %s %s\n", err->row - 1, err->lines[0],
                err->truncated[0] ? "[...]" : "");
    fprintf(stream, "%4d | %s %s <------ Error is here!\n", err->row, err->lines[1],
            err->truncated[1] ? "[...]" : "");
    fprintf(stream, "%4d | %s %s\n", err->row + 1, err->lines[2],
            err->truncated[2] ? "[...]" : "");
    fprintf(stream, "\n");
#endif
}

static int
error(CfgError *err, const char *fmt, ...)
{
    const char prefix[] = "";  // "CfgError: ";
    const int prefix_len = sizeof(prefix) - 1;

    err->off = cur();
    err->row = 1;
    err->col = 1;
    for (int i = 0; i < cur(); i++) {
        err->col++;
        if (scanner.src[i] == '\n') {
            err->row++;
            err->col = 1;
        }
    }

#ifdef CFG_DETAILED_ERRORS
    {
        const char *src = scanner.src;

        // Get line offset containing the error's location
        int line_off = cur();

        while (line_off > 0) {
            if (src[line_off - 1] == '\n')
                break;
            line_off--;
        }

        int line_len = 0;
        for (int i = line_off; i < scanner.len; i++) {
            if (src[i] == '\n') {
                if (i > 0 && src[i - 1] == '\r')
                    line_len--;
                break;
            }
            line_len++;
        }

        int prev_line_off;
        int prev_line_len;

        if (line_off == 0) {
            prev_line_off = 0;
            prev_line_len = 0;
        } else {
            prev_line_off = line_off - 1;
            while (prev_line_off > 0) {
                if (src[prev_line_off - 1] == '\n')
                    break;
                prev_line_off--;
            }
            prev_line_len = line_off - prev_line_off - 1;
        }

        int next_line_off;
        int next_line_len;

        if (line_off + line_len == scanner.len) {
            next_line_off = 0;
            next_line_len = 0;
        } else {
            next_line_off = line_off + line_len + 1;
            if (src[next_line_off] == '\n')
                next_line_off++;
            next_line_len = 0;
            while (next_line_off + next_line_len < scanner.len &&
                   src[next_line_off + next_line_len] != '\n')
                next_line_len++;
        }

        err->truncated[0] = false;
        err->truncated[1] = false;
        err->truncated[2] = false;

        if (prev_line_len >= (int) sizeof(err->lines[0])) {
            prev_line_len = (int) sizeof(err->lines[0]) - 1;
            err->truncated[0] = true;
        }

        if (line_len >= (int) sizeof(err->lines[1])) {
            line_len = (int) sizeof(err->lines[1]) - 1;
            err->truncated[1] = true;
        }

        if (next_line_len >= (int) sizeof(err->lines[2])) {
            next_line_len = (int) sizeof(err->lines[2]) - 1;
            err->truncated[2] = true;
        }

        memcpy(err->lines[0], src + prev_line_off, prev_line_len);
        memcpy(err->lines[1], src + line_off, line_len);
        memcpy(err->lines[2], src + next_line_off, next_line_len);
        err->lines[0][prev_line_len] = '\0';
        err->lines[1][line_len] = '\0';
        err->lines[2][next_line_len] = '\0';
    }
#endif

    va_list vargs;
    va_start(vargs, fmt);
    snprintf(err->msg, CFG_MAX_ERR + 1, prefix);
    vsnprintf(err->msg + prefix_len, CFG_MAX_ERR - prefix_len + 1, fmt, vargs);
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

static void
copy_slice_into(int src_off, int src_len, char *dst, int max)
{
    assert(src_len < max);
    memcpy(dst, scanner.src + src_off, src_len);
    dst[src_len] = '\0';
}

static int
parse_string(CfgEntry *entry, CfgError *err)
{
    // Consume opening '"'
    advance();

    int val_offset = cur();
    while (!is_at_end() && is_string(peek()))
        advance();

    if (is_at_end() || peek() != '"')
        return error(err, "closing '\"' expected");

    int val_len = cur() - val_offset;
    if (val_len > CFG_MAX_VAL)
        return error(err, "value too long");

    // Consume closing '"'
    advance();

    copy_slice_into(val_offset, val_len, entry->val.str, sizeof(entry->val.str));
    entry->type = TYPE_STR;
    return 0;
}

static int
parse_true(CfgEntry *entry, CfgError *err)
{
    if (!follows_string(cur(), "true", 4))
        return error(err, "invalid literal");

    // Consume "true"
    advance2(4);

    entry->val.bool_ = true;
    entry->type = TYPE_BOOL;
    return 0;
}

static int
parse_false(CfgEntry *entry, CfgError *err)
{
    if (!follows_string(cur(), "false", 5))
        return error(err, "invalid literal");

    // Consume "false"
    advance2(5);

    entry->val.bool_ = false;
    entry->type = TYPE_BOOL;
    return 0;
}

static int
parse_rgba(CfgEntry *entry, CfgError *err)
{
    if (!follows_string(cur(), "rgba", 4))
        return error(err, "invalid literal");

    // Consume "rgba"
    advance2(4);

    // Skip whitespace between 'a' and '('
    skip_blank();

    if (is_at_end() || peek() != '(')
        return error(err, "'(' expected");

    // Consume '('
    advance();

    bool is_int;
    uint8_t rgb[3];
    for (int i = 0; i < 3; i++) {
        // Skip whitespace preceding the number
        skip_blank();

        float number = consume_number(&is_int);
        if (!is_int || number < 0 || number > 255)
            return error(err, "invalid number");
        rgb[i] = (uint8_t) number;

        // Skip whitespace following the number
        skip_blank();

        if (is_at_end() || peek() != ',')
            return error(err, "',' expected");

        // Consume ','
        advance();
    }

    // Skip whitespace preceding the alpha
    skip_blank();

    float alpha = consume_number(&is_int);

    if (alpha < 0 || alpha > 1)
        return error(err, "invalid number");

    // Skip whitespace following alpha
    skip_blank();

    if (is_at_end() || peek() != ')')
        return error(err, "')' expected");

    // Consume ')'
    advance();

    CfgColor color = {
        .r = rgb[0],
        .g = rgb[1],
        .b = rgb[2],
        .a = (uint8_t) (alpha * 255),
    };
    entry->val.color = color;
    entry->type = TYPE_COLOR;
    return 0;
}

static int
parse_literal(CfgEntry *entry, CfgError *err)
{
    int code;
    switch (peek()) {
    case 't':
        code = parse_true(entry, err);
        break;
    case 'f':
        code = parse_false(entry, err);
        break;
    case 'r':
        code = parse_rgba(entry, err);
        break;
    default:
        return error(err, "invalid literal");
    }
    return code;
}

static void
parse_number(CfgEntry *entry)
{
    bool is_int;
    float number = consume_number(&is_int);

    if (is_int) {
        entry->val.int_ = (int) number;
        entry->type = TYPE_INT;
    } else {
        entry->val.float_ = number;
        entry->type = TYPE_FLOAT;
    }
}

static int
parse_value(CfgEntry *entry, CfgError *err)
{
    // Skip blank space between ':' and value
    skip_blank();

    // Missing value
    if (is_at_end() || peek() == '\n')
        return error(err, "missing value");

    char c = peek();

    int code;

    if (c == '"')
        code = parse_string(entry, err);
    else if (isalpha(c))
        code = parse_literal(entry, err);
    else if (isdigit(c) || (c == '-' && isdigit(peek_next()))) {
        parse_number(entry);
        code = 0;
    } else
        code = error(err, "invalid value");

    return code;
}

static int
parse_key(CfgEntry *entry, CfgError *err)
{
    // Missing key
    if (is_at_end() || !is_key(peek()))
        return error(err, "missing key");

    // Consume key
    int key_offset = cur();
    do
        advance();
    while (!is_at_end() && is_key(peek()));
    int key_len = cur() - key_offset;

    if (key_len > CFG_MAX_KEY)
        return error(err, "key too long");

    copy_slice_into(key_offset, key_len, entry->key, sizeof(entry->key));
    return 0;
}

static int
consume_key_and_value_separator(CfgError *err)
{
    // Skip blank space between the key and ':'
    skip_blank();

    if (is_at_end() || peek() != ':')
        return error(err, "':' expected");

    // Consume ':'
    advance();
    return 0;
}

static int
parse_entry(CfgEntry *entry, CfgError *err)
{
    if (parse_key(entry, err))
        return -1;

    if (consume_key_and_value_separator(err))
        return -1;

    // Consume value
    if (parse_value(entry, err) < 0)
        return -1;

    // Go to the end of the line and consume the \n
    // (if the file ended that's okay too)
    skip_blank();
    if (!is_at_end()) {
        if (peek() != '\n')
            return error(err, "unexpected character '%c'", peek());
        advance();
    }

    return 0;
}

static void
init_error(CfgError *err)
{
    err->off = -1;
    err->col = -1;
    err->row = -1;
    err->msg[0] = '\0';
}

int
cfg_parse(const char *src, int src_len, Cfg *cfg, CfgError *err)
{
    init_error(err);
    init_scanner(src, src_len);

    cfg->size = 0;

    // Skip initial whitespace and comments
    skip_whitespace_and_comments();
    while (!is_at_end() && cfg->size < cfg->max_entries) {
        CfgEntry *entry = &cfg->entries[cfg->size];
        if (parse_entry(entry, err) < 0)
            return -1;
        cfg->size++;
        skip_whitespace_and_comments();
    }
    return 0;
}

static char *
load_file_bytes(const char *filename, int *len, char *msg)
{
    FILE *file = fopen(filename, "rb");
    if (!file) {
        snprintf(msg, CFG_MAX_ERR + 1, "failed to open the file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t size = (size_t) ftell(file);
    rewind(file);

    char *src = malloc(size + 1);
    if (src == NULL) {
        snprintf(msg, CFG_MAX_ERR + 1, "memory allocation failed");
        return NULL;
    }

    size_t bytes_read = fread(src, sizeof(char), size, file);
    fclose(file);

    if (bytes_read != size) {
        free(src);
        snprintf(msg, CFG_MAX_ERR + 1, "failed to read the file");
        return NULL;
    }

    *len = size;
    return src;
}

int
cfg_load(const char *filename, Cfg *cfg, CfgError *err)
{
    init_error(err);

    char *ext = strrchr(filename, '.');
    if (strcmp(ext, ".cfg") != 0) {
        snprintf(err->msg, CFG_MAX_ERR + 1, "invalid file extension");
        return -1;
    }

    int len;
    char *src;

    src = load_file_bytes(filename, &len, err->msg);
    if (src == NULL)
        return -1;

    int res = cfg_parse(src, len, cfg, err);

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

CfgColor
cfg_get_color(Cfg cfg, const char *key, CfgColor default_)
{
    return *(CfgColor *) get_val(cfg, key, &default_, TYPE_COLOR);
}

void
cfg_fprint(FILE *stream, Cfg cfg)
{
    for (int i = 0; i < cfg.size; i++) {
        fprintf(stream, "%s: ", cfg.entries[i].key);

        switch (cfg.entries[i].type) {
        case TYPE_STR:
            fprintf(stream, "\"%s\"\n", cfg.entries[i].val.str);
            break;
        case TYPE_BOOL:
            fprintf(stream, "%s\n", cfg.entries[i].val.bool_ ? "true" : "false");
            break;
        case TYPE_INT:
            fprintf(stream, "%d\n", cfg.entries[i].val.int_);
            break;
        case TYPE_FLOAT:
            fprintf(stream, "%f\n", cfg.entries[i].val.float_);
            break;
        case TYPE_COLOR:;
            CfgColor c = cfg.entries[i].val.color;
            fprintf(stream, "rgba(%d, %d, %d, %d)\n", c.r, c.g, c.b, c.a);
            break;
        default:
            fprintf(stream, "CfgError: unknown type\n");
        }
    }
}
