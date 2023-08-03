#ifndef CONFIG_H
#define CONFIG_H

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

int cfg_load(const char *filename, CfgEntry *entries);
int cfg_get(const char *key, CfgEntry *entries);

int parse(const char *src,
          int len,  // src_len
          CfgEntry *entries,
          int max_entries,
          char *err);  // int err_len

#endif

// Approximate EBNF Grammar

// cfg  ::= line*
// line ::= key ":" val "\n"
// key  ::= str
// val  ::= str | int | float

// str   ::= alpha+
// alpha ::= "a" ... "z" | "A" ... "Z"

// int   ::= digit+
// float ::= digit+ "." digit+
// digit ::= "0" ... "9"
