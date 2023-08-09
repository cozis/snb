#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>
#include "widget.h"

typedef void (*TableIterFuncStart)(void *context);
typedef void (*TableIterFuncEnd  )(void *context);
typedef bool (*TableIterFuncNext )(void *context);
typedef void (*TableIterFuncField)(void *context, int index, char *dst, size_t max);

typedef struct {
    TableIterFuncStart start;
    TableIterFuncField field;
    TableIterFuncNext  next;
    TableIterFuncEnd   end;
} TableFunctions;

typedef struct {
    float entry_h;
    float pad_h;
    float pad_v;
    Color background_active;
    const char *font_file;
    float       font_size;
    Color       font_color;
    Color       font_active;
} TableStyle;

#define MAX_TABLE_COLUMNS 8
#define MAX_TABLE_COLUMN_LABEL 32

typedef void (*TableCallback)(void *context, int index);

typedef struct {
    Widget base;
    TableStyle *style;

    void *context;
    TableFunctions funcs;
    TableCallback  callback;

    Font        loaded_font;
    const char *loaded_font_file;
    float       loaded_font_size;

    char  column_labels[MAX_TABLE_COLUMNS][MAX_TABLE_COLUMN_LABEL];
    float column_width[MAX_TABLE_COLUMNS];
    int   num_columns;
    int   num_rows;

    int active;
} TableView;

bool initTableView(TableView *table, WidgetStyle *base_style, TableStyle *style, void *context, TableFunctions funcs, TableCallback callback);
void tableViewChanged(TableView *table);
void setColumnLabel(TableView *table, int index, const char *label);

#endif