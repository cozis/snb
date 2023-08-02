#include <raylib.h>
#include "widget.h"
#include "gap_buffer.h"

typedef struct {
    float line_h;
    float cursor_w;
    float ruler_x;
    float pad_h;
    float pad_v;
    int spaces_per_tab;
    Color color_cursor;
    Color color_text;
    Color color_ruler;
    const char *font_path;
    float       font_size;
} BufferViewStyle;

typedef struct {
    Widget base;
    BufferViewStyle *style;
    const char *loaded_font_path;
    float       loaded_font_size;
    Font        loaded_font;
    bool        selecting;
    size_t      select_first;
    size_t      select_second;
    GapBuffer *gap;
    char file[1024];
} BufferView;

Widget *createBufferView(BufferViewStyle *style);