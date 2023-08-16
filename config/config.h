#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define CONFIG_FILE_EXT ".cfg"

#define CONFIG_MAX_KEY 32
#define CONFIG_MAX_VAL 64
#define CONFIG_MAX_ERR 64

typedef struct {
    int off;
    int col;
    int row;
    char msg[CONFIG_MAX_ERR];
} ConfigError;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} ConfigColor;

typedef enum {
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_BOOL,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_FLOAT,
    CONFIG_TYPE_COLOR,
} ConfigValType;

typedef union {
    char string[CONFIG_MAX_VAL + 1];
    bool boolean;
    int integer;
    float floating;
    ConfigColor color;
} ConfigVal;

typedef struct {
    ConfigValType type;
    char key[CONFIG_MAX_KEY + 1];
    ConfigVal val;
} ConfigEntry;

// Returns the number of entries if successful, -1 otherwise
int Config_parse(const char *src,
                 int src_len,
                 ConfigEntry *entries,
                 int max_entries,
                 ConfigError *err);

// Returns the number of entries if successful, -1 otherwise
int Config_parseFile(const char *filename,
                     ConfigEntry *entries,
                     int max_entries,
                     ConfigError *err);

ConfigEntry *Config_getEntry(ConfigEntry *entries, int count, const char *key);

void config_fprint_error(FILE *stream, ConfigError *err);

#endif
