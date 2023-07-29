#include <raylib.h>
#include "gap_buffer.h"

typedef struct {
    float line_h;
    float cursor_w;
    Color color_cursor;
    Color color_text;
    const char *font_path;
    float       font_size;
} BufferViewStyle;

typedef struct {
    BufferViewStyle *style;
    const char *loaded_font_path;
    float       loaded_font_size;
    Font        loaded_font;
    GapBuffer *gap;
} BufferView;

void initBufferView(BufferView *bufview, BufferViewStyle *style, GapBuffer *gap);
void freeBufferView(BufferView *bufview);
void drawBufferView(BufferView *bufview);
