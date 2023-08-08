#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_KEY_LEN 32
#define MAX_VAL_LEN 64
#define MAX_ERR_LEN 64

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    float a;
} CfgColor;

typedef enum {
    TYPE_STR,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_COLOR,
} CfgValType;

typedef union {
    char str[MAX_VAL_LEN + 1];
    bool bool_;
    int int_;
    float float_;
    CfgColor color;
} CfgVal;

typedef struct {
    CfgValType type;
    char key[MAX_KEY_LEN + 1];
    CfgVal val;
} CfgEntry;

typedef struct {
    CfgEntry *entries;
    int max_entries;
    int size;
} Cfg;

/**
 * @brief Initializes a Cfg object
 *
 * @param[out] cfg The Cfg object to be initialized
 * @param[in] entries The array that will contain the parsed entries
 * @param[in] max_entries The maximum number of entries the array can hold
 */
void cfg_init(Cfg *cfg, CfgEntry *entries, int max_entries);

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
int cfg_parse(const char *src, int src_len, Cfg *cfg, char *err);

/**
 * @brief Loads and parses a config file
 *
 * @param[in] filename Path of the config file
 * @param[in,out] cfg The Cfg object to be populated
 * @param[out] err Buffer to store error messages
 *
 * @return 0 if loading and parsing are successful, -1 otherwise
 *
 * @see cfg_parse() The underlying function that this function wraps
 */
int cfg_load(const char *filename, Cfg *cfg, char *err);

char *cfg_get_str(Cfg cfg, const char *key, char *default_);
bool cfg_get_bool(Cfg cfg, const char *key, bool default_);
int cfg_get_int(Cfg cfg, const char *key, int default_);
float cfg_get_float(Cfg cfg, const char *key, float default_);
CfgColor cfg_get_color(Cfg cfg, const char *key, CfgColor default_);

void cfg_print(Cfg cfg);

#endif
