#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_KEY_LEN 32
#define MAX_VAL_LEN 32

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

void
remove_dup_ws(char *str)
{
    int j = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isblank(str[i]) || (i > 0 && !isblank(str[i - 1])))
            str[j++] = str[i];
    }
    str[j] = '\0';
}

int
is_allowed(char ch)
{
    return isalnum(ch) || isblank(ch) || ispunct(ch);
}

int
parse(const char *src, int len, CfgEntry *entries, int max_entries)
{
    int i = 0;
    int cur = 0;
    int count = 0;

    // Consume leading whitespace
    while (cur < len && isspace(src[cur]))
        cur++;

    while (cur < len && count < max_entries) {
        // Missing key
        if (cur == len || !isalpha(src[cur]))
            return -1;

        // Consume key
        i = 0;
        while (cur < len && i < MAX_KEY_LEN && isalpha(src[cur]))
            entries[count].key[i++] = src[cur++];

        entries[count].key[i++] = '\0';

        // Consume whitespace between the key and ':'
        while (cur < len && isspace(src[cur]))
            cur++;

        if (cur == len || src[cur] != ':')
            return -1;

        // Consume ':'
        cur++;

        // Consume whitespace between ':' and value
        while (cur < len && isspace(src[cur]))
            cur++;

        // Missing value
        if (cur == len)
            return -1;

        // Consume value
        char c = src[cur];

        if (isalpha(c) || ispunct(c)) {
            entries[count].type = TYPE_STR;
            i = 0;

            // Copy all the value
            while (cur < len && i < MAX_VAL_LEN && is_allowed(src[cur]))
                entries[count].val.str[i++] = src[cur++];
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

            while (cur < len && isdigit(src[cur]))
                int_part = int_part * 10 + (src[cur++] - '0');

            if (src[cur] == '.') {
                cur++;
                is_float = 1;
            }

            if (is_float) {
                int div = 1;
                while (cur < len && isdigit(src[cur])) {
                    fract_part = fract_part * 10 + (src[cur++] - '0');
                    div *= 10;
                }

                entries[count].type = TYPE_FLOAT;
                entries[count].val.float_ = int_part + (fract_part / div);
            } else {
                entries[count].type = TYPE_INT;
                entries[count].val.int_ = int_part;
            }
        } else {
            return -1;
        }

        count++;

        // Consume whitespace for the next entry
        while (cur < len && isspace(src[cur]))
            cur++;
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

    char src[size + 1];
    size_t bytes_read = fread(src, sizeof(char), size, file);
    fclose(file);

    if (bytes_read != size) {
        fprintf(stderr, "Error: failed to read the file\n");
        return 1;
    }

    src[size] = '\0';

    const int max_entries = 32;
    CfgEntry entries[max_entries];

    // Q: Why not strlen(src) + 1 (for null-terminator)
    int num_entries = parse(src, strlen(src), entries, max_entries);

    if (num_entries < 0) {
        fprintf(stderr, "Error: failed to parse the input\n");
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
