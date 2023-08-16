#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <raylib.h>
#include <string.h>
#include "../utils/basic.h"
#include "button.h"

static void handleEvent(Widget *widget, Event event);
static Vector2 draw(Widget *widget, Vector2 offset, Vector2 area);
static void free_(Widget *widget);

static void reloadFont(Button *button)
{
    const char *font_file = button->style->font_file;
    float       font_size = button->style->font_size;

    Font font;
    if (button->style->font_file)
        font = LoadFontEx(font_file, font_size, NULL, 250);
    else
        font = GetFontDefault();

    UnloadFont(button->loaded_font); // FIXME: Is it okay to unload the default font?
    button->loaded_font = font;
    button->loaded_font_file = font_file;
    button->loaded_font_size = font_size;
}

static void reloadStyleIfChanged(Button *button)
{
    if (button->style) {
        bool changed_font_file = (button->style->font_file != button->loaded_font_file);
        bool changed_font_size = (button->style->font_size != button->loaded_font_size);
        if (changed_font_file || changed_font_size)
            reloadFont(button);
    }
}

Button *createButton(WidgetStyle *base_style, ButtonStyle *style, const char *label, 
                     void *context, ButtonCallback callback)
{
    Button *button = malloc(sizeof(Button));
    if (button == NULL)
        return NULL;

    initWidget(&button->base, base_style, draw, free_, handleEvent);

    button->style = style;
    button->active = false;
    button->context = context;
    button->callback = callback;
    button->loaded_font = GetFontDefault();
    button->loaded_font_file = NULL;
    button->loaded_font_size = style->font_size;

    strncpy(button->label, label, MAX_BUTTON_LABEL);
    button->label[MAX_BUTTON_LABEL-1] = '\0';

    return button;
}

static void free_(Widget *widget)
{
    Button *button = (Button*) widget;
    UnloadFont(button->loaded_font);
    free(button);
}

static void handleEvent(Widget *widget, Event event)
{
    Button *button = (Button*) widget;
    switch (event.type) {
        case EVENT_MOUSE_LEFT_DOWN:
        button->active = true;
        setMouseFocus(widget);
        break;

        case EVENT_MOUSE_LEFT_UP:
        button->active = false;
        setMouseFocus(NULL);
        if (button->callback) 
            button->callback(button->context, button);
        break;

        default:
        break;
    }
}

static Vector2 draw(Widget *widget, Vector2 offset, Vector2 area)
{
    Button *button = (Button*) widget;
    
    reloadStyleIfChanged(button);

    bool active = button->active;

    Vector2 logic_area;

    // Draw the background of the button
    {
        Color color_background;
        if (active)
            color_background = button->style->color_background_active;
        else
            color_background = button->style->color_background;

        float roundness = button->style->roundness;
        int    segments = button->style->segments;
        
        Rectangle rect;
        rect.x = offset.x;
        rect.y = offset.y;
        rect.width  = area.x;
        rect.height = area.y;
        DrawRectangleRounded(rect, roundness, segments, color_background);
    }

    // Draw the text of the button
    {
        Color color_text;
        if (active)
            color_text = button->style->color_text_active;
        else
            color_text = button->style->color_text;
        
        const char *text = button->label;
        Font        font = button->loaded_font;
        float  font_size = button->loaded_font_size;
        float    spacing = 0;

        Vector2 text_area = MeasureTextEx(font, text, font_size, spacing);

        Vector2 text_pos  = {
            .x = offset.x + (area.x - text_area.x) / 2,
            .y = offset.y + (area.y - text_area.y) / 2,
        };
        
        DrawTextEx(font, text, text_pos, font_size, spacing, color_text);

        logic_area = text_area;
    }

    return logic_area;
}
