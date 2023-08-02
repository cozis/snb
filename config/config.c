#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEY_LEN 32
#define MAX_VAL_LEN 32
#define MAX_ERR_LEN 64

typedef enum { TYPE_STR, TYPE_INT, TYPE_FLOAT } CfgValType;

typedef struct {
    char key[MAX_KEY_LEN + 1];
    CfgValType type;
    union {
        char str[MAX_VAL_LEN + 1];
        int int_;
        float float_;
    } val;
} CfgEntry;

typedef struct {
    const char *src;
    int cur;
    int len;
} Scanner;

static void
remove_dup_ws(char *str)
{
    int j = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isblank(str[i]) || (i > 0 && !isblank(str[i - 1])))
            str[j++] = str[i];
    }
    str[j] = '\0';
}

static int
is_key(char ch)
{
    return isalpha(ch) || ch == '.';
}

static int
is_value(char ch)
{
    return isalnum(ch) || isblank(ch) || ispunct(ch);
}

static int
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
parse(Scanner *scanner, CfgEntry *entries, int max_entries, char *err)
{
    int i = 0;
    int count = 0;

    // Skip leading whitespace
    skip_whitespace(scanner);

    while (!is_at_end(scanner) && count < max_entries) {
        // Missing key
        if (is_at_end(scanner) || !is_key(peek(scanner))) {
            char *fmt = "Error: missing key in entry %d";
            snprintf(err, MAX_ERR_LEN + 1, fmt, count + 1);
            return -1;
        }

        // Consume key
        i = 0;
        while (!is_at_end(scanner) && i < MAX_KEY_LEN && is_key(peek(scanner)))
            entries[count].key[i++] = advance(scanner);

        entries[count].key[i++] = '\0';

        // Skip whitespace between the key and ':'
        skip_whitespace(scanner);

        if (is_at_end(scanner) || peek(scanner) != ':') {
            char *fmt = "Error: ':' expected in entry %d";
            snprintf(err, MAX_ERR_LEN + 1, fmt, count + 1);
            return -1;
        }

        // Consume ':'
        advance(scanner);

        // Skip whitespace between ':' and value
        skip_whitespace(scanner);

        // Missing value
        if (is_at_end(scanner) || peek(scanner) == '\n') {
            char *fmt = "Error: missing value in entry %d";
            snprintf(err, MAX_ERR_LEN + 1, fmt, count + 1);
            return -1;
        }

        // Consume value
        char c = peek(scanner);

        if (isalpha(c) || ispunct(c)) {
            entries[count].type = TYPE_STR;
            i = 0;

            // Copy all the value
            while (!is_at_end(scanner) && i < MAX_VAL_LEN &&
                   is_value(peek(scanner)))
                entries[count].val.str[i++] = advance(scanner);
            i--;

            // Remove trailing whitespace
            while (i > 0 && isblank(entries[count].val.str[i]))
                i--;

            entries[count].val.str[++i] = '\0';
            remove_dup_ws(entries[count].val.str);
        } else if (isdigit(c)) {
            int is_float = 0;
            int int_part = 0;
            float fract_part = 0;

            while (!is_at_end(scanner) && isdigit(peek(scanner)))
                int_part = int_part * 10 + (advance(scanner) - '0');

            if (!is_at_end(scanner) && peek(scanner) == '.') {
                advance(scanner);
                is_float = 1;
            }

            if (is_float) {
                int div = 1;
                while (!is_at_end(scanner) && isdigit(peek(scanner))) {
                    fract_part = fract_part * 10 + (advance(scanner) - '0');
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
            snprintf(err, MAX_ERR_LEN + 1, fmt, count + 1);
            return -1;
        }

        count++;

        // Skip whitespace for the next entry
        skip_whitespace(scanner);
    }

    return count;
}

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
    Scanner scanner = {.src = src, .cur = 0, .len = strlen(src)};
    int num_entries = parse(&scanner, entries, max_entries, err);

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

// EBNF Grammar

// cfg  ::= line*
// line ::= key ":" val "\n"
// key  ::= str
// val  ::= str | int | float

// str   ::= alpha+
// alpha ::= "a" ... "z" | "A" ... "Z"

// int   ::= digit+
// float ::= digit+ "." digit+
// digit ::= "0" ... "9"

// **************************************** //

// Compile with: gcc -Wall -Wextra config.c
// Run with: ./a.out config.cfg
