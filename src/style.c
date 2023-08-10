#include <stdlib.h> // abort
#include "style.h"
#include "utils/basic.h"
//#include "widget/text_input.h"
//#include "widget/split_view.h"

static WidgetStyle     base_style;
static WidgetStyle     base_input_style;
static WidgetStyle     base_table_style;
static BufferViewStyle style;
static ButtonStyle     button_style;
static TextInputStyle  input_style;
static TableStyle      table_style;

void initStyle(void)
{
    base_style = (WidgetStyle) {

        .show_scrollbar_h = true,
        .show_scrollbar_v = true,
        
        .roundness = 0,
        .segments  = 0,

        .color_background = (Color) {0x19, 0x1b, 0x27, 0xff},

        .scrollbar_thumb_roundness = 0.5,
        .scrollbar_thumb_segments  = 5,
        .scrollbar_thumb_width     = 15,
        .scrollbar_thumb_margin    = 5,

        .scrollbar_color              = (Color) {0x0f, 0x10, 0x16, 0xff},
        .scrollbar_track_color        = (Color) {0x46, 0x49, 0x56, 0xff},
        .scrollbar_thumb_color        = LIGHTGRAY,
        .scrollbar_thumb_active_color = (Color) {0x39, 0x82, 0x38, 0xff},
    };

    style = (BufferViewStyle) {
        .line_h   = 1,
        .ruler_x  = 80,
        .cursor_w = 3,
        .pad_h    = 10,
        .pad_v    = 10,
        .color_cursor = RED,
        .color_text   = LIGHTGRAY,
        .color_ruler  = (Color) {0x46, 0x49, 0x56, 0xff},
        .font_path = "SourceCodePro-Regular.ttf",
        .font_size = 24,
        .spaces_per_tab = 8,
    };

    button_style = (ButtonStyle) {
        .roundness = 0.3,
        .segments  = 10,
        .color_text = (Color) {0xcc, 0xcc, 0xcc, 0xff},
        .color_text_active = (Color) {0x33, 0x33, 0x33, 0xff},
        .color_background = (Color) {0x40, 0x40, 0x40, 0xff},
        .color_background_active = (Color) {0x83, 0xc5, 0xbf, 0xff},
        .font_size = 24,
        .font_file = "SourceCodePro-Regular.ttf",
    };

    base_input_style = base_style;
    base_input_style.roundness = 0.3;
    base_input_style.segments  = 10;
    base_input_style.color_background = (Color) {0x46, 0x49, 0x56, 0xff};
    base_input_style.show_scrollbar_h = false;

    base_table_style = base_style;
    base_table_style.roundness = 0;
    base_table_style.segments  = 0;
    base_table_style.color_background = (Color) {0x46, 0x49, 0x56, 0xff};

    input_style = (TextInputStyle) {
        .line_h   = 1,
        .cursor_w = 3,
        .pad_h    = 3,
        .pad_v    = 3,
        .color_cursor = RED,
        .color_text   = LIGHTGRAY,
        .font_path = "SourceCodePro-Regular.ttf",
        .font_size = 24,
        .spaces_per_tab = 8,
    };

    table_style = (TableStyle) {
        .entry_h = 25,
        .pad_h = 5,
        .pad_v = 5,
        .font_file = "SourceCodePro-Regular.ttf",
        .font_size = 25,
        .font_color = BLACK,
        .font_active = RED,
        .background_active = GREEN,
    };
}

void freeStyle(void)
{
}

TextInput *createStylizedTextInput(void)
{
    TextInput *input = createTextInput(&base_input_style, &input_style);
    if (input == NULL)
        abort();
    return input;
}

Button *createStylizedButton(const char *label, void *context, 
                            ButtonCallback callback)
{
    Button *button = createButton(&base_style, &button_style, label, context, callback);
    if (button == NULL)
        abort();
    return button;
}

GroupView *createStylizedGroupView(void)
{
    GroupView *group = createGroupView(&base_style);
    if (group == NULL)
        abort();
    return group;
}

void initStylizedTableView(TableView *table, void *context, 
                           TableFunctions iter_funcs, TableCallback callback)
{
    if (!initTableView(table, &base_table_style, &table_style, context, iter_funcs, callback))
        abort();
}

BufferView *createStylizedBufferView(void)
{
    BufferView *buff = createBufferView(&base_style, &style);
    if (buff == NULL)
        abort();
    return buff;   
}

void stylizedSplitView(SplitDirection dir, Widget *first, Widget *second)
{
    if (!splitView(&base_style, dir, first, second))
        abort();
}

#define MIN_FONT_SIZE 14
#define MAX_FONT_SIZE 100
#define INC_FONT_SIZE 2

void increaseFontSize(void)
{
    style.font_size = MIN(style.font_size + INC_FONT_SIZE, MAX_FONT_SIZE);
}

void decreaseFontSize(void)
{
    style.font_size = MAX(style.font_size - INC_FONT_SIZE, MIN_FONT_SIZE);
}
