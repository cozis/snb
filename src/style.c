#include <stdlib.h> // abort
#include <assert.h>
#include "style.h"
#include "utils/basic.h"
#include "utils/config.h"

static WidgetStyle     base_style;
static WidgetStyle     base_input_style;
static WidgetStyle     base_table_style;
static BufferViewStyle style;
static ButtonStyle     button_style;
static TextInputStyle  input_style;
static TableStyle      table_style;

#define MAX_CONFIG_PARAMS 512

static CfgEntry params[MAX_CONFIG_PARAMS];
static Cfg config;

static bool getParamBool(const char *name, bool fallback)
{
    return cfg_get_bool(config, name, fallback);
}

/*
static int getParamInt(const char *name, int fallback)
{
    return cfg_get_int(config, name, fallback);
}
*/

static int getParamIntMin(const char *name, int fallback, int min)
{
    assert(fallback >= min);
    int value = cfg_get_int(config, name, fallback);
    if (value < min) value = fallback;
    return value;
}

/*
static float getParamFloat(const char *name, float fallback)
{
    return cfg_get_float(&config, name, fallback);
}
*/

static float getParamFloatMin(const char *name, float fallback, float min)
{
    assert(fallback >= min);
    float value = cfg_get_float(config, name, fallback);
    if (value < min) value = fallback;
    return value;
}

static float getParamFloatRange(const char *name, float fallback, float min, float max)
{
    assert(fallback >= min && fallback <= max);
    float value = cfg_get_float(config, name, fallback);
    if (value < min || value > max) value = fallback;
    return value;
}

static Color getParamColor(const char *name, Color fallback)
{
    CfgColor fallback2 = {
        .r=fallback.r, 
        .g=fallback.g, 
        .b=fallback.b, 
        .a=fallback.a
    };
    CfgColor value = cfg_get_color(config, name, fallback2);
    Color value2 = {
        .r=value.r, 
        .g=value.g, 
        .b=value.b, 
        .a=value.a
    };
    return value2;
}

static const char *getParamString(const char *name, const char *fallback)
{
    return cfg_get_str(config, name, fallback);
}

static void applyStyle(void)
{
    base_style = (WidgetStyle) {

        .show_scrollbar_h = getParamBool("widget.scrollbar.show.h", true),
        .show_scrollbar_v = getParamBool("widget.scrollbar.show.v", true),
        
        .roundness = getParamFloatRange("widget.roundness", 0, 0, 1),
        .segments  = getParamIntMin("widget.segments", 0, 0),

        .color_background = getParamColor("widget.background", (Color) {0x19, 0x1b, 0x27, 0xff}),

        .scrollbar_thumb_roundness = getParamFloatRange("widget.scrollbar.thumb.roundness", 0.5, 0, 1),
        .scrollbar_thumb_segments  = getParamIntMin("widget.scrollbar.thumb.segments", 5, 0),
        .scrollbar_thumb_width     = getParamIntMin("widget.scrollbar.thumb.width", 15, 0),
        .scrollbar_thumb_margin    = getParamIntMin("widget.scrollbar.thumb.margin", 5, 0),

        .scrollbar_color              = getParamColor("widget.scrollbar.background", (Color) {0x0f, 0x10, 0x16, 0xff}),
        .scrollbar_track_color        = getParamColor("widget.scrollbar.track.color", (Color) {0x46, 0x49, 0x56, 0xff}),
        .scrollbar_thumb_color        = getParamColor("widget.scrollbar.thumb.color", LIGHTGRAY),
        .scrollbar_thumb_active_color = getParamColor("widget.scrollbar.thumb.active", (Color) {0x39, 0x82, 0x38, 0xff}),
    };

    style = (BufferViewStyle) {
        .line_h   = getParamFloatMin("buffer.line.h", 1, 0.1),

        .ruler_x  = getParamIntMin("buffer.ruler.x", 80, 0),
        .color_ruler  = getParamColor("buffer.ruler.color", (Color) {0x46, 0x49, 0x56, 0xff}),

        .pad_h    = getParamIntMin("buffer.padding.h", 10, 0),
        .pad_v    = getParamIntMin("buffer.padding.w", 10, 0),
        
        .cursor_w = getParamIntMin("buffer.cursor.w", 3, 1),
        .color_cursor = getParamColor("buffer.cursor.color", RED),
        .color_text   = getParamColor("buffer.text.color", LIGHTGRAY),
        
        .font_file = getParamString("buffer.font.path", "SourceCodePro-Regular.ttf"),
        .font_size = getParamIntMin("buffer.font.size", 24, 0),
        .spaces_per_tab = getParamIntMin("buffer.spaces_per_tab", 8, 1),
    };

    button_style = (ButtonStyle) {
        .roundness = getParamFloatRange("button.roundness", 0.3, 0, 1),
        .segments  = getParamIntMin("button.segments", 10, 0),
        .color_text = getParamColor("button.text.color", (Color) {0xcc, 0xcc, 0xcc, 0xff}),
        .color_text_active = getParamColor("button.text.active", (Color) {0x33, 0x33, 0x33, 0xff}),
        .color_background = getParamColor("button.background", (Color) {0x40, 0x40, 0x40, 0xff}),
        .color_background_active = getParamColor("button.background.active", (Color) {0x83, 0xc5, 0xbf, 0xff}),
        .font_size = getParamIntMin("button.font.size", 24, 0),
        .font_file = getParamString("button.font.file", "SourceCodePro-Regular.ttf"),
    };

    base_input_style = base_style;
    base_input_style.roundness = getParamFloatRange("widget_input.roundness", 0.3, 0, 1);
    base_input_style.segments  = getParamIntMin("widget_input.segments", 10, 0);
    base_input_style.color_background = getParamColor("widget_input.background", (Color) {0x46, 0x49, 0x56, 0xff});
    base_input_style.show_scrollbar_h = getParamBool("widget_input.scrollbar.show.h", false);

    base_table_style = base_style;
    base_table_style.roundness = getParamFloatRange("widget_table.roundness", 0, 0, 1);
    base_table_style.segments  = getParamIntMin("widget_table.segments", 0, 0);
    base_table_style.color_background = getParamColor("widget_table.background", (Color) {0x46, 0x49, 0x56, 0xff});

    input_style = (TextInputStyle) {
        .line_h   = getParamFloatMin("text_input.line.h", 1, 0.1),
        .cursor_w = getParamIntMin("text_input.cursor.w", 3, 1),
        .pad_h    = getParamIntMin("text_input.padding.h", 3, 0),
        .pad_v    = getParamIntMin("text_input.padding.v", 3, 0),
        .color_cursor = getParamColor("text_input.cursor.color", RED),
        .color_text   = getParamColor("text_input.text.color", LIGHTGRAY),
        .font_file = getParamString("text_input.font.file", "SourceCodePro-Regular.ttf"),
        .font_size = getParamIntMin("text_input.font.size", 24, 0),
        .spaces_per_tab = getParamIntMin("text_input.spaces_per_tab", 8, 1),
    };

    table_style = (TableStyle) {
        .entry_h = getParamIntMin("table.entry.h", 25, 0),
        .pad_h = getParamIntMin("table.padding.h", 5, 0),
        .pad_v = getParamIntMin("table.padding.v", 5, 0),
        .font_file = getParamString("table.font.file", "SourceCodePro-Regular.ttf"),
        .font_size = getParamIntMin("table.font.size", 25, 0),
        .font_color = getParamColor("table.font.color", BLACK),
        .font_active = getParamColor("table.font.active", RED),
        .background_active = getParamColor("table.entry.active", GREEN),
    };
}

void loadStyleFrom(const char *file)
{
    CfgError error;
    cfg_init(&config, params, MAX_CONFIG_PARAMS);
    if (cfg_load(file, &config, &error))
        cfg_fprint_error(stderr, &error);
    applyStyle();
}

void initStyle(void)
{
    cfg_init(&config, params, MAX_CONFIG_PARAMS);
    applyStyle();
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
