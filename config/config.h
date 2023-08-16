#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// #define CFG_DETAILED_ERRORS

#define CFG_FILE_EXT ".cfg"

#define CFG_MAX_KEY 32
#define CFG_MAX_VAL 64
#define CFG_MAX_ERR 64

typedef struct {
    int off;
    int col;
    int row;
    char msg[CFG_MAX_ERR];

#ifdef CFG_DETAILED_ERRORS
    bool truncated[3];
    char lines[3][64];
#endif

} CfgError;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} CfgColor;

typedef enum {
    CFG_TYPE_STR,
    CFG_TYPE_BOOL,
    CFG_TYPE_INT,
    CFG_TYPE_FLOAT,
    CFG_TYPE_COLOR,
} CfgValType;

typedef union {
    char str[CFG_MAX_VAL + 1];
    bool bool_;
    int int_;
    float float_;
    CfgColor color;
} CfgVal;

typedef struct {
    CfgValType type;
    char key[CFG_MAX_KEY + 1];
    CfgVal val;
} CfgEntry;

typedef struct {
    CfgEntry *entries;
    int count;
    int capacity;
} Cfg;

/**
 * @brief Parses the source data and populates the Cfg object
 *
 * @param[in] src The source data
 * @param[in] src_len Length of the source data
 * @param[in,out] cfg The Cfg object to be populated
 * @param[out] err Buffer to store error messages
 *
 * @return 0 if parsing is successful, -1 otherwise
 */
int cfg_parse(const char *src, int src_len, Cfg *cfg, CfgError *err);

/**
 * @brief Loads and parses a config file
 *
 * @param[in] filename Path of the config file
 * @param[in,out] cfg The Cfg object to be populated
 * @param[out] err Buffer to store error messages
 *
 * @return 0 if loading and parsing are successful, -1 otherwise
 *
 * @see cfg_parse() The underlying "wrapped" function
 */
int cfg_load(const char *filename, Cfg *cfg, CfgError *err);

char *cfg_get_str(Cfg *cfg, const char *key, char *default_);

bool cfg_get_bool(Cfg *cfg, const char *key, bool default_);

int cfg_get_int(Cfg *cfg, const char *key, int default_);
int cfg_get_int_min(Cfg *cfg, const char *key, int default_, int min);
int cfg_get_int_max(Cfg *cfg, const char *key, int default_, int max);
int cfg_get_int_range(Cfg *cfg, const char *key, int default_, int min, int max);

float cfg_get_float(Cfg *cfg, const char *key, float default_);
float cfg_get_float_min(Cfg *cfg, const char *key, float default_, float min);
float cfg_get_float_max(Cfg *cfg, const char *key, float default_, float max);
float cfg_get_float_range(Cfg *cfg,
                          const char *key,
                          float default_,
                          float min,
                          float max);

CfgColor cfg_get_color(Cfg *cfg, const char *key, CfgColor default_);

void cfg_fprint(FILE *stream, Cfg *cfg);
void cfg_fprint_error(FILE *stream, CfgError *err);

#endif
