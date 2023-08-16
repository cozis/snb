#ifndef TEXT_INPUT_H
#define TEXT_INPUT_H

#include "widget.h"
#include "../utils/gap_buffer.h"

typedef struct {
    float line_h;
    float cursor_w;
    float pad_h;
    float pad_v;
    int spaces_per_tab;
    Color color_cursor;
    Color color_text;
    const char *font_file;
    float       font_size;
} TextInputStyle;

typedef struct {
    Widget base;
    TextInputStyle *style;
    const char *loaded_font_file;
    float       loaded_font_size;
    Font        loaded_font;
    bool        selecting;
    size_t      select_first;
    size_t      select_second;
    GapBuffer *gap;
} TextInput;

TextInput *createTextInput(WidgetStyle *base_style, TextInputStyle *style);
size_t     getTextInputContents(TextInput *input, char *dst, size_t max);
void       setTextInputContents(TextInput *input, const char *path);
#endif