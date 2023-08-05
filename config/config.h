#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define MAX_KEY_LEN 32
#define MAX_VAL_LEN 64
#define MAX_ERR_LEN 64

typedef enum { TYPE_STR, TYPE_BOOL, TYPE_INT, TYPE_FLOAT } CfgValType;

typedef struct {
    char key[MAX_KEY_LEN + 1];
    CfgValType type;
    union {
        char str[MAX_VAL_LEN + 1];
        bool bool_;
        int int_;
        float float_;
    } val;
} CfgEntry;

typedef struct {
    CfgEntry *entries;
    int max_entries;
    int size;
} Cfg;

void cfg_init(Cfg *cfg, CfgEntry *entries, int max_entries);

int cfg_parse(const char *src, int src_len, Cfg *cfg, char *err);
int cfg_load(const char *filename, Cfg *cfg, char *err);

int cfg_get_int(Cfg cfg, const char *key, int default_);
float cfg_get_float(Cfg cfg, const char *key, float default_);
char *cfg_get_str(Cfg cfg, const char *key, char *default_);

void cfg_print(Cfg cfg);

#endif

// EBNF Grammar

// cfg   ::= line*
// line  ::= key ':' val '\n'
// key   ::= (alpha | '.')+
// val   ::= str | bool | int | float

// str   ::= '"' (alpha | punct | digit | blank)+ '"'
// alpha ::= 'a' ... 'z' | 'A' ... 'Z'
// punct ::= '.' | ':' | '~' | '!' | ...
// blank ::= ' ' | '\t'

// bool  ::= true | false

// int   ::= '-'? digit+
// float ::= '-'? digit+ '.' digit+
// digit ::= '0' ... '9'
