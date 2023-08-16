#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "text_input.h"
#include "../utils/basic.h"

size_t getTextInputContents(TextInput *input, char *dst, size_t max)
{
    GapBuffer *gap = input->gap;
    if (dst && max > 0)
        GapBuffer_copyDataOut(gap, dst, max);
    return GapBuffer_getByteCount(gap);
}

void setTextInputContents(TextInput *input, const char *path)
{
    GapBuffer *gap = input->gap;
    GapBuffer_whipeClean(gap);
    GapBuffer_insertString(gap, path, strlen(path));
}

static int UTF8ToUTF32(const char *utf8_data, int nbytes, uint32_t *utf32_code)
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

static void handleEvent(Widget *widget, Event event);
static Vector2 draw(Widget *widget, Vector2 offset, Vector2 area);
static void free_(Widget *widget);

TextInput *createTextInput(WidgetStyle *base_style, TextInputStyle *style)
{
    TextInput *input = malloc(sizeof(TextInput));
    if (input == NULL)
        return NULL;

    GapBuffer *gap;
    {
        size_t len = 1 << 16;
        void  *mem = malloc(len);
        if (mem == NULL) {
            free(input);
            return NULL;
        }
        gap = GapBuffer_createUsingMemory(mem, len, free);
        if (gap == NULL) {
            free(input);
            free(mem);
            return NULL;
        }
    }

    initWidget(&input->base, base_style, draw, free_, handleEvent);
    input->style = style;
    input->loaded_font_file = NULL;
    input->loaded_font_size = 14;
    input->loaded_font = GetFontDefault();
    input->selecting = false;
    input->select_first  = 0;
    input->select_second = 0;
    input->gap = gap;

    return input;
}

static void free_(Widget *widget)
{
    TextInput *input = (TextInput*) widget;
    UnloadFont(input->loaded_font);
    GapBuffer_destroy(input->gap);
    free(input);
}

static void reloadFont(TextInput *input)
{
    const char *font_file = input->style->font_file;
    float       font_size = input->style->font_size;

    Font font;
    if (input->style->font_file)
        font = LoadFontEx(font_file, font_size, NULL, 250);
    else
        font = GetFontDefault();

    UnloadFont(input->loaded_font); // FIXME: Is it okay to unload the default font?
    input->loaded_font = font;
    input->loaded_font_file = font_file;
    input->loaded_font_size = font_size;
}

static void reloadStyleIfChanged(TextInput *input)
{
    if (input->style) {
        bool changed_font_file = (input->style->font_file != input->loaded_font_file);
        bool changed_font_size = (input->style->font_size != input->loaded_font_size);
        if (changed_font_file || changed_font_size)
            reloadFont(input);
    }
}

static bool somethingSelected(TextInput *input)
{
    return input->select_first != input->select_second;
}

static void dropSelection(TextInput *input)
{
    input->select_first  = 0;
    input->select_second = 0;
}

static void drawSelection(TextInput *input, GapBufferLine line, 
                          float line_x, float line_y, 
                          float line_h, size_t line_offset)
{
    if (!somethingSelected(input))
        return;
        
    size_t select_start = MIN(input->select_first, input->select_second);
    size_t select_end   = MAX(input->select_first, input->select_second);

    if (select_start >= line_offset + line.len || select_end < line_offset)
        return;

    size_t select_in_line_start;
    size_t select_in_line_end;

    if (select_start < line_offset)
        select_in_line_start = 0;
    else
        select_in_line_start = select_start - line_offset;

    if (select_end < line_offset + line.len)
        select_in_line_end = select_end - line_offset;
    else
        select_in_line_end = line.len;

    Font       font = input->loaded_font;
    float font_size = input->loaded_font_size;

    Rectangle selection_rect = {
        .x = line_x + stringRenderWidth(font, font_size, line.str, select_in_line_start),
        .y = line_y,
        .width  = stringRenderWidth(font, font_size, line.str + select_in_line_start, select_in_line_end - select_in_line_start),
        .height = line_h,
    };
    DrawRectangleRec(selection_rect, (Color) {0x34, 0x37, 0x45, 0xff});
}

static Vector2 draw(Widget *widget, Vector2 offset, Vector2 area)
{
    (void) area;

    TextInput *input = (TextInput*) widget;
    reloadStyleIfChanged(input);

    float font_size    = input->style->font_size;
    float line_h       = input->style->line_h * font_size;
    float cursor_w     = input->style->cursor_w;
    float pad_h        = input->style->pad_h;
    float pad_v        = input->style->pad_v;
    Color cursor_color = input->style->color_cursor;
    Color   font_color = input->style->color_text;

    if (getFocus() != widget)
        cursor_color = GRAY;

    Font      font = input->loaded_font;
    GapBuffer *gap = input->gap;
    
    size_t cursor = GapBuffer_rawCursorPosition(gap);

    GapBufferLine line;
    GapBufferIter iter;
    GapBufferIter_init(&iter, gap);

    Vector2 logic_area = {0, 0};

    int line_x = offset.x + pad_h;
    int line_y = offset.y + pad_v;
    bool   drew_cursor = false;
    size_t line_offset = 0;
    size_t  line_count = 0;
    while (GapBufferIter_next(&iter, &line)) {

        drawSelection(input, line, line_x, line_y, line_h, line_offset);
        
        float line_w = renderString(font, line.str, line.len, 
                                    line_x, line_y, font_size, 
                                    font_color);
        
        logic_area.x = MAX(logic_area.x, 2*pad_h + line_w);

        if (cursor >= line_offset && cursor <= line_offset + line.len) {
            int relative_cursor_x = stringRenderWidth(font, font_size, line.str, cursor - line_offset);
            DrawRectangle(line_x + relative_cursor_x, line_y, cursor_w, line_h, cursor_color);
            drew_cursor = true;
            line_w += cursor_w;
        }

        line_y += line_h;
        line_offset += line.len + 1; // line.len doesn't count the \n
        line_count++;
    }
    GapBufferIter_free(&iter);

    if (!drew_cursor) {
        DrawRectangle(line_x, line_y, cursor_w, line_h, cursor_color);
        line_y += line_h;
        line_count++;
    }

    logic_area.y = 2*pad_v + line_count * line_h;
    return logic_area;
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

static size_t 
getOffsetAssociatedToCoordinates(TextInput *input, 
                                 Vector2 point)
{
    GapBuffer *gap = input->gap;

    float font_size = input->loaded_font_size;
    float pad_h     = input->style->pad_h;
    float pad_v     = input->style->pad_v;

    int line_index = (point.y - pad_v) / (input->style->line_h * font_size);

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
        cursor = line_offset + longestSubstringThatRendersInLessPixelsThan(input->loaded_font, font_size, // This function name is too long..
                                                                           line.str, line.len, point.x - pad_h);
    else
        // If the line index is out of bounds, then the line offset
        // will be the number of bytes in the file, which is an out
        // of bounds index.
        cursor = MIN(line_offset, GapBuffer_getByteCount(gap));
    return cursor;
}

static void manageClick(TextInput *input, Vector2 mouse)
{
    GapBuffer *gap = input->gap;

    size_t cursor = getOffsetAssociatedToCoordinates(input, mouse);
    GapBuffer_moveAbsoluteRaw(gap, cursor);

    input->selecting = true;
    input->select_first  = cursor;
    input->select_second = cursor;
    setMouseFocus((Widget*) input);
}

static void removeSelectionAndMoveCursorThere(TextInput *input)
{
    if (somethingSelected(input)) {

        GapBuffer *gap = input->gap;

        size_t select_start = MIN(input->select_first, input->select_second);
        size_t select_end   = MAX(input->select_first, input->select_second);
                
        size_t num_selected_bytes = select_end - select_start;
        GapBuffer_moveAbsolute(gap, input->select_first);
        GapBuffer_removeForwardsRaw(gap, num_selected_bytes);

        dropSelection(input);
        input->selecting = false;
    }
}

#define MAX_SPACES_PER_TAB 32

static void insertTab(TextInput *input)
{
    GapBuffer *gap = input->gap;
    
    int spaces_per_tab = input->style->spaces_per_tab;
    spaces_per_tab = MAX(spaces_per_tab, 0);
    spaces_per_tab = MIN(spaces_per_tab, MAX_SPACES_PER_TAB);

    char spaces[MAX_SPACES_PER_TAB];

    int num = spaces_per_tab - GapBuffer_getColumn(gap) % spaces_per_tab;
    memset(spaces, ' ', num);

    if (!GapBuffer_insertString(gap, spaces, num))
        fprintf(stderr, "Couldn't insert tab\n");
}

static void manageKey(TextInput *input, int key)
{
    GapBuffer *gap = input->gap;

    switch (key) {
        
        case KEY_UP:
        dropSelection(input);
        GapBuffer_moveRelativeVertically(gap, true);
        break;
        
        case KEY_DOWN:
        dropSelection(input);
        GapBuffer_moveRelativeVertically(gap, false);
        break;
        
        case KEY_LEFT:
        dropSelection(input);
        GapBuffer_moveRelative(gap, -1); 
        break;
        
        case KEY_RIGHT: 
        dropSelection(input);
        GapBuffer_moveRelative(gap, +1); 
        break;

        case KEY_ENTER:
        break;
        
        case KEY_BACKSPACE:
        if (somethingSelected(input))
            removeSelectionAndMoveCursorThere(input);
        else
            GapBuffer_removeBackwards(gap, 1); 
        break;
        
        case KEY_DELETE:
        if (somethingSelected(input))
            removeSelectionAndMoveCursorThere(input);
        else
            GapBuffer_removeForwards(gap, 1);
        break;

        case KEY_TAB:
        if (somethingSelected(input))
            removeSelectionAndMoveCursorThere(input);
        else
            insertTab(input); 
        break;
    }
}

static void handleEvent(Widget *widget, Event event)
{
    TextInput *input = (TextInput*) widget;
    GapBuffer *gap = input->gap;

    switch (event.type) {

        case EVENT_MOUSE_LEFT_DOWN:
        SetWindowTitle("SnB");
        setFocus(widget);
        manageClick(input, event.mouse);
        break;

        case EVENT_MOUSE_LEFT_UP:
        if (input->selecting) {
            setMouseFocus(NULL);
            input->selecting = false;
        }
        break;

        case EVENT_MOUSE_MOVE:
        if (input->selecting) {
            input->select_second = getOffsetAssociatedToCoordinates(input, event.mouse);
            GapBuffer_moveAbsoluteRaw(gap, input->select_second);
        }
        break;

        case EVENT_TEXT:
        removeSelectionAndMoveCursorThere(input);
        if (!GapBuffer_insertRune(gap, event.rune)) 
            fprintf(stderr, "Couldn't insert string\n");
        break;

        case EVENT_KEY:
        manageKey(input, event.key);
        break;

        default:
        break;
    }
}
