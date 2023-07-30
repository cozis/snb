#include <assert.h>
#include <stdint.h>
#include "buff_view.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

int UTF8ToUTF32(const char *utf8_data, int nbytes, uint32_t *utf32_code)
{
    assert(utf8_data != NULL);
    assert(utf32_code != NULL);
    assert(nbytes >= 0);

    if(nbytes == 0)
        return -1;

    if(utf8_data[0] & 0x80) {

        // May be UTF-8.
            
        if((unsigned char) utf8_data[0] >= 0xF0) {
            // 4 bytes.
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

            if(nbytes < 4)
                return -1;
                    
            uint32_t temp 
                = (((uint32_t) utf8_data[0] & 0x07) << 18) 
                | (((uint32_t) utf8_data[1] & 0x3f) << 12)
                | (((uint32_t) utf8_data[2] & 0x3f) <<  6)
                | (((uint32_t) utf8_data[3] & 0x3f));

            if(temp > 0x10ffff)
                return -1;

            *utf32_code = temp;
            return 4;
        }
            
        if((unsigned char) utf8_data[0] >= 0xE0) {
            // 3 bytes.
            // 1110xxxx 10xxxxxx 10xxxxxx

            if(nbytes < 3)
                return -1;

            uint32_t temp
                = (((uint32_t) utf8_data[0] & 0x0f) << 12)
                | (((uint32_t) utf8_data[1] & 0x3f) <<  6)
                | (((uint32_t) utf8_data[2] & 0x3f));
                    
            if(temp > 0x10ffff)
                return -1;

            *utf32_code = temp;
            return 3;
        }
            
        if((unsigned char) utf8_data[0] >= 0xC0) {
            // 2 bytes.
            // 110xxxxx 10xxxxxx

            if(nbytes < 2)
                return -1;

            *utf32_code 
                = (((uint32_t) utf8_data[0] & 0x1f) << 6)
                | (((uint32_t) utf8_data[1] & 0x3f));
                    
            assert(*utf32_code <= 0x10ffff);
            return 2;
        }
        
        return -1;
    }

    // It's ASCII
    // 0xxxxxxx

    *utf32_code = (uint32_t) utf8_data[0];
    return 1;
}

static float 
renderString(Font font, const char *str, size_t len,
             int off_x, int off_y, float font_size, 
             Color tint)
{
    if (font.texture.id == 0) 
        font = GetFontDefault();

    int   y = off_y;
    float x = off_x; // Offset X to next character to draw

    float spacing = 0;
    float scale = (float) font_size / font.baseSize; // Character quad scaling factor

    size_t i = 0;
    while (i < len) {

        uint32_t codepoint;
        int consumed = UTF8ToUTF32(str + i, len - i, &codepoint);

        if (consumed < 1) {
            codepoint = '?';
            consumed = 1;
        }

        assert(consumed > 0);
        int glyph_index = GetGlyphIndex(font, codepoint);

        assert(codepoint != '\n');

        if (codepoint != ' ' && codepoint != '\t') {
            Vector2 position = {x, y};
            DrawTextCodepoint(font, codepoint, position, 
                              font_size, tint);
        }

        float delta;
        int advance_x = font.glyphs[glyph_index].advanceX;
        if (advance_x == 0)
            delta = (float) font.recs[glyph_index].width * scale + spacing;
        else 
            delta = (float) advance_x * scale + spacing;

        x += delta;
        i += consumed;
    }
    float w = x - (float) off_x;
    return w;
}

static float 
stringRenderWidth(Font font, int font_size, const char *str, size_t len)
{
    if (font.texture.id == 0) 
        font = GetFontDefault();

    float scale = (float) font_size / font.baseSize;
    float  w = 0;
    size_t i = 0;
    while (i < len) {

        int consumed;
        int letter = GetCodepoint(str + i, &consumed);
        assert(letter != '\n');

        int glyph_index = GetGlyphIndex(font, letter);

        if (letter == 0x3f)
            i++;
        else
            i += consumed;

        float delta;
        int advanceX = font.glyphs[glyph_index].advanceX;
        if (advanceX)
            delta = (float) advanceX * scale;
        else
            delta = (float) font.recs[glyph_index].width * scale
                  + (float) font.glyphs[glyph_index].offsetX * scale;

        w += delta;
    }
    return w;
}

void initBufferView(BufferView *bufview, BufferViewStyle *style, GapBuffer *gap)
{
    bufview->style = style;
    bufview->loaded_font_path = NULL;
    bufview->loaded_font_size = 14;
    bufview->loaded_font = GetFontDefault();
    bufview->gap = gap;
}

void freeBufferView(BufferView *bufview)
{
    (void) bufview;
}

static void reloadFont(BufferView *bufview)
{
    const char *font_path = bufview->style->font_path;
    float       font_size = bufview->style->font_size;

    Font font;
    if (bufview->style->font_path)
        font = LoadFontEx(font_path, font_size, NULL, 250);
    else
        font = GetFontDefault();

    UnloadFont(bufview->loaded_font); // FIXME: Is it okay to unload the default font?
    bufview->loaded_font = font;
    bufview->loaded_font_path = font_path;
    bufview->loaded_font_size = font_size;
}

static void reloadStyleIfChanged(BufferView *bufview)
{
    if (bufview->style) {
        bool changed_font_path = (bufview->style->font_path != bufview->loaded_font_path);
        bool changed_font_size = (bufview->style->font_size != bufview->loaded_font_size);
        if (changed_font_path || changed_font_size)
            reloadFont(bufview);
    }
}

static void drawRuler(Font font, int font_size, int ruler_width, Color color) 
{
    float font_width = MeasureTextEx(font, "A", font_size, 0).x;
    int x = ruler_width * font_width;
    DrawLine(x, 0, x, GetScreenHeight(), color);
}

void drawBufferView(BufferView *bufview)
{
    reloadStyleIfChanged(bufview);

    float font_size    = bufview->style->font_size;
    float line_h       = bufview->style->line_h * font_size;
    float cursor_w     = bufview->style->cursor_w;
    float ruler_x      = bufview->style->ruler_x;
    Color cursor_color = bufview->style->color_cursor;
    Color   font_color = bufview->style->color_text;
    Color  ruler_color = bufview->style->color_ruler;

    Font      font = bufview->loaded_font;
    GapBuffer *gap = bufview->gap;
    
    size_t cursor = GapBuffer_rawCursorPosition(gap);

    drawRuler(font, font_size, ruler_x, ruler_color);

    GapBufferLine line;
    GapBufferIter iter;
    GapBufferIter_init(&iter, gap);

    int line_x = 0;
    int line_y = 0;
    size_t line_offset = 0;
    size_t line_count = 0;
    bool drew_cursor = false;
    while (GapBufferIter_next(&iter, &line)) {
            
        float line_w = renderString(font, line.str, line.len, 
                                    line_x, line_y, font_size, 
                                    font_color);

        if (cursor >= line_offset && cursor <= line_offset + line.len) {
            int relative_cursor_x = stringRenderWidth(font, font_size, line.str, cursor - line_offset);
            DrawRectangle(line_x + relative_cursor_x, line_y, cursor_w, line_h, cursor_color);
            drew_cursor = true;
            line_w += cursor_w;
        }

        line_y += line_h;
        line_offset += line.len + 1;
        line_count++;
    }
    GapBufferIter_free(&iter);

    if (!drew_cursor) {
        DrawRectangle(line_x, line_y, cursor_w, line_h, cursor_color);
        line_count++;
    }
}

static size_t 
longestSubstringThatRendersInLessPixelsThan(Font font, int font_size,
                                            const char *str, size_t len, 
                                            float max_px_len)
{
    if (str == NULL)
        str = "";

    if (font.texture.id == 0) 
        font = GetFontDefault();

    float scale = (float) font_size / font.baseSize;

    float  w = 0;
    size_t i = 0;
    while (i < len) {

        uint32_t codepoint;
        int consumed = UTF8ToUTF32(str + i, len - i, &codepoint);

        if (consumed < 0) {
            codepoint = '?';
            consumed = 1;
        }
        i += consumed;
        int glyph_index = GetGlyphIndex(font, codepoint);

        float delta;
        int advanceX = font.glyphs[glyph_index].advanceX;
        if (advanceX)
            delta = (float) advanceX * scale;
        else
            delta = (float) font.recs[glyph_index].width * scale
                  + (float) font.glyphs[glyph_index].offsetX * scale;
        
        assert(delta >= 0);
        if (w + delta > max_px_len)
            break;

        w += delta;
    }
    assert(i <= len);
    return i;
}

void clickBufferView(BufferView *bufview, Vector2 mouse)
{
    GapBuffer *gap = bufview->gap;

    float font_size = bufview->loaded_font_size;

    int line_index = mouse.y / (bufview->style->line_h * font_size);

    GapBufferLine line;
    GapBufferIter iter;
    GapBufferIter_init(&iter, gap);
    bool not_last = true;
    int  index = 0;
    size_t line_offset = 0;
    while (index < line_index && (not_last = GapBufferIter_next(&iter, &line))) {
        index++;
        line_offset += line.len + 1;
    }

    size_t cursor;
    if (not_last && GapBufferIter_next(&iter, &line))
        cursor = line_offset + longestSubstringThatRendersInLessPixelsThan(bufview->loaded_font, font_size, 
                                                                           line.str, line.len, mouse.x);
    else
        // If the line index is out of bounds, then the line offset
        // will be the number of bytes in the file, which is an out
        // of bounds index.
        cursor = MIN(line_offset, GapBuffer_getByteCount(gap));
    GapBuffer_moveAbsoluteRaw(gap, cursor);
}