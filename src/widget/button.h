#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"

#define MAX_BUTTON_LABEL 128

typedef struct Button Button;

typedef void (*ButtonCallback)(void *context, Button *button);

typedef struct {
    float roundness;
    int   segments;
    Color color_text;
    Color color_text_active;
    Color color_background;
    Color color_background_active;
    float       font_size;
    const char *font_file;
} ButtonStyle;

struct Button {
    Widget base;
    ButtonStyle *style;
    void *context;
    ButtonCallback callback;
    bool active;
    Font        loaded_font;
    const char *loaded_font_file;
    float       loaded_font_size;
    char label[MAX_BUTTON_LABEL];
};

Button *createButton(WidgetStyle *base_style, ButtonStyle *style, const char *label, void *context, ButtonCallback callback);

#endif