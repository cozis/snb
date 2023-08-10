#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "../utils/basic.h"
#include "../spawn_dialog.h"
#include "buff_view.h"

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

#define MAX_BUFFERS 32

static void handleEvent(Widget *widget, Event event);
static Vector2 draw(Widget *widget, Vector2 offset, Vector2 area);
static void free_(Widget *widget);

static bool initialized = false;
static BufferView buffers[MAX_BUFFERS];
static bool  used_buffers[MAX_BUFFERS];

static BufferView *allocStructMemory(void)
{
    if (!initialized) {
        for (int i = 0; i < MAX_BUFFERS; i++)
            used_buffers[i] = false;
        initialized = true;
    }

    BufferView *bufview = NULL;
    for (int i = 0; i < MAX_BUFFERS; i++)
        if (used_buffers[i] == false) {
            bufview = buffers+i;
            used_buffers[i] = true;
            break;
        }

    return bufview;
}

static void freeStructMemory(BufferView *bufview)
{
    int i = bufview - buffers;
    assert(i >= 0 && i < MAX_BUFFERS && used_buffers[i] == true);
    used_buffers[i] = false;
}

BufferView *createBufferView(WidgetStyle *base_style, BufferViewStyle *style)
{
    BufferView *bufview = allocStructMemory();

    GapBuffer *gap;
    {
        size_t len = 1 << 16;
        void  *mem = malloc(len);
        if (mem == NULL) {
            freeStructMemory(bufview);
            return NULL;
        }
        gap = GapBuffer_createUsingMemory(mem, len, free);
        if (gap == NULL) {
            freeStructMemory(bufview);
            free(mem);
            return NULL;
        }
    }

    initWidget(&bufview->base, base_style, draw, free_, handleEvent);
    bufview->style = style;
    bufview->loaded_font_file = NULL;
    bufview->loaded_font_size = 14;
    bufview->loaded_font = GetFontDefault();
    bufview->selecting = false;
    bufview->select_first  = 0;
    bufview->select_second = 0;
    bufview->gap = gap;
    bufview->file[0] = '\0';

    return bufview;
}

static void free_(Widget *widget)
{
    BufferView *bufview = (BufferView*) widget;
    UnloadFont(bufview->loaded_font);
    GapBuffer_destroy(bufview->gap);
    freeStructMemory(bufview);
}

static void reloadFont(BufferView *bufview)
{
    const char *font_file = bufview->style->font_file;
    float       font_size = bufview->style->font_size;

    Font font;
    if (bufview->style->font_file)
        font = LoadFontEx(font_file, font_size, NULL, 250);
    else
        font = GetFontDefault();

    UnloadFont(bufview->loaded_font); // FIXME: Is it okay to unload the default font?
    bufview->loaded_font = font;
    bufview->loaded_font_file = font_file;
    bufview->loaded_font_size = font_size;
}

static void reloadStyleIfChanged(BufferView *bufview)
{
    if (bufview->style) {
        bool changed_font_file = (bufview->style->font_file != bufview->loaded_font_file);
        bool changed_font_size = (bufview->style->font_size != bufview->loaded_font_size);
        if (changed_font_file || changed_font_size)
            reloadFont(bufview);
    }
}

static void drawRuler(float x, float y, float h, Font font, int font_size, int ruler_width, Color color) 
{
    float font_width = MeasureTextEx(font, "A", font_size, 0).x;
    int offset = ruler_width * font_width;
    DrawLine(x + offset, y, x + offset, y + h, color);
}

static bool somethingSelected(BufferView *bufview)
{
    return bufview->select_first != bufview->select_second;
}

static void dropSelection(BufferView *bufview)
{
    bufview->select_first  = 0;
    bufview->select_second = 0;
}

static void drawSelection(BufferView *bufview, GapBufferLine line, 
                          float line_x, float line_y, 
                          float line_h, size_t line_offset)
{
    if (!somethingSelected(bufview))
        return;
        
    size_t select_start = MIN(bufview->select_first, bufview->select_second);
    size_t select_end   = MAX(bufview->select_first, bufview->select_second);

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

    Font       font = bufview->loaded_font;
    float font_size = bufview->loaded_font_size;

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

    BufferView *bufview = (BufferView*) widget;
    reloadStyleIfChanged(bufview);

    float font_size    = bufview->style->font_size;
    float line_h       = bufview->style->line_h * font_size;
    float cursor_w     = bufview->style->cursor_w;
    float ruler_x      = bufview->style->ruler_x;
    float pad_h        = bufview->style->pad_h;
    float pad_v        = bufview->style->pad_v;
    Color cursor_color = bufview->style->color_cursor;
    Color   font_color = bufview->style->color_text;
    Color  ruler_color = bufview->style->color_ruler;

    if (getFocus() != widget)
        cursor_color = GRAY;

    Font      font = bufview->loaded_font;
    GapBuffer *gap = bufview->gap;
    
    size_t cursor = GapBuffer_rawCursorPosition(gap);

    drawRuler(offset.x, offset.y, bufview->base.last_logic_area.y, font, font_size, ruler_x, ruler_color);

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

        drawSelection(bufview, line, line_x, line_y, line_h, line_offset);
        
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
getOffsetAssociatedToCoordinates(BufferView *bufview, 
                                 Vector2 point)
{
    GapBuffer *gap = bufview->gap;

    float font_size = bufview->loaded_font_size;
    float pad_h     = bufview->style->pad_h;
    float pad_v     = bufview->style->pad_v;

    int line_index = (point.y - pad_v) / (bufview->style->line_h * font_size);

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
        cursor = line_offset + longestSubstringThatRendersInLessPixelsThan(bufview->loaded_font, font_size, // This function name is too long..
                                                                           line.str, line.len, point.x - pad_h);
    else
        // If the line index is out of bounds, then the line offset
        // will be the number of bytes in the file, which is an out
        // of bounds index.
        cursor = MIN(line_offset, GapBuffer_getByteCount(gap));
    return cursor;
}

static void manageClick(BufferView *bufview, Vector2 mouse)
{
    GapBuffer *gap = bufview->gap;

    size_t cursor = getOffsetAssociatedToCoordinates(bufview, mouse);
    GapBuffer_moveAbsoluteRaw(gap, cursor);

    bufview->selecting = true;
    bufview->select_first  = cursor;
    bufview->select_second = cursor;
    setMouseFocus((Widget*) bufview);
}

static void removeSelectionAndMoveCursorThere(BufferView *bufview)
{
    if (somethingSelected(bufview)) {

        GapBuffer *gap = bufview->gap;

        size_t select_start = MIN(bufview->select_first, bufview->select_second);
        size_t select_end   = MAX(bufview->select_first, bufview->select_second);
                
        size_t num_selected_bytes = select_end - select_start;
        GapBuffer_moveAbsolute(gap, bufview->select_first);
        GapBuffer_removeForwardsRaw(gap, num_selected_bytes);

        dropSelection(bufview);
        bufview->selecting = false;
    }
}

#define MAX_SPACES_PER_TAB 32

static void insertTab(BufferView *bufview)
{
    GapBuffer *gap = bufview->gap;
    
    int spaces_per_tab = bufview->style->spaces_per_tab;
    spaces_per_tab = MAX(spaces_per_tab, 0);
    spaces_per_tab = MIN(spaces_per_tab, MAX_SPACES_PER_TAB);

    char spaces[MAX_SPACES_PER_TAB];

    int num = spaces_per_tab - GapBuffer_getColumn(gap) % spaces_per_tab;
    memset(spaces, ' ', num);

    if (!GapBuffer_insertString(gap, spaces, num))
        fprintf(stderr, "Couldn't insert tab\n");
}

static void manageKey(BufferView *bufview, int key)
{
    GapBuffer *gap = bufview->gap;

    switch (key) {
        
        case KEY_UP:
        dropSelection(bufview);
        GapBuffer_moveRelativeVertically(gap, true);
        break;
        
        case KEY_DOWN:
        dropSelection(bufview);
        GapBuffer_moveRelativeVertically(gap, false);
        break;
        
        case KEY_LEFT:
        dropSelection(bufview);
        GapBuffer_moveRelative(gap, -1); 
        break;
        
        case KEY_RIGHT: 
        dropSelection(bufview);
        GapBuffer_moveRelative(gap, +1); 
        break;

        case KEY_ENTER:
        removeSelectionAndMoveCursorThere(bufview);
        if (!GapBuffer_insertString(gap, "\n", 1))
            fprintf(stderr, "Couldn't insert string\n");
        break;
        
        case KEY_BACKSPACE:
        if (somethingSelected(bufview))
            removeSelectionAndMoveCursorThere(bufview);
        else
            GapBuffer_removeBackwards(gap, 1); 
        break;
        
        case KEY_DELETE:
        if (somethingSelected(bufview))
            removeSelectionAndMoveCursorThere(bufview);
        else
            GapBuffer_removeForwards(gap, 1);
        break;

        case KEY_TAB:
        if (somethingSelected(bufview))
            removeSelectionAndMoveCursorThere(bufview);
        else
            insertTab(bufview); 
        break;
    }
}

static void changeWindowTitle(BufferView *bufview)
{
    const char *file;
    if (bufview->file[0])
        file = bufview->file;
    else
        file = "(unnamed)";

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "SnB - %s", file);
    SetWindowTitle(buffer);
}

static void changeWindowTitleIfFocused(BufferView *bufview)
{
    if (getFocus() == (Widget*) bufview)
        changeWindowTitle(bufview);
}

static void openFile(BufferView *bufview, const char *filename)
{
    assert(filename);
    
    size_t filename_len = strlen(filename);

    if (filename_len >= sizeof(bufview->file)) {
        fprintf(stderr, "File path is too long (longer than the buffer file path limit)\n");
        return;
    }

    // Try and open the file into a new gap buffer
    size_t mem_len = 1 << 16; 
    void *mem = malloc(mem_len);
    if (mem == NULL) {
        fprintf(stderr, "Failed to allocate gap buffer memory to load file\n");
        return;
    }
    GapBuffer *gap = GapBuffer_createUsingMemory(mem, mem_len, free);
    if (gap == NULL) {
        fprintf(stderr, "Failed to initialize gap buffer to load file\n");
        free(mem);
        return;
    }

    if (!GapBuffer_insertFile(gap, filename)) {
        
        fprintf(stderr, "Failed to load '%s' into gap buffer (file too big or not valid utf-8)\n", filename);
        
        // Free the new gap buffer
        GapBuffer_destroy(gap);

    } else {

        strcpy(bufview->file, filename);
        changeWindowTitleIfFocused(bufview);

        fprintf(stderr, "Loaded '%s'\n", filename);

        // Swap the old gap buffer with the new one
        GapBuffer_destroy(bufview->gap);
        bufview->gap = gap;
    }
}

static bool generateRandomFilename(char *dst, size_t max)
{
    size_t len = MIN(16, max);

    char table[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890";
    
    for (size_t i = 0; i < len-1; i++) {
        int k = rand() % (sizeof(table)-1);
        dst[i] = table[k];
    }
    dst[len-1] = '\0';

    // TODO: Make sure this file doesn't exist
    return true;
}

static void saveFile(BufferView *bufview)
{
    if (bufview->file[0] == '\0') {
        int n = chooseFileToSave(bufview->file, sizeof(bufview->file));
        if (n <= 0)
            return;
    }

    // Save to a secondary file
    char second[1024];
    if (!generateRandomFilename(second, sizeof(second)))
        return;

    if (!GapBuffer_saveTo(bufview->gap, second)) {
        fprintf(stderr, "Couldn't save data to file '%s'\n", second);
        return;
    }

    // Data was written succesfully to the secondary
    // file so now we can swap it with the actual
    // target file.
    remove(bufview->file);
    rename(second, bufview->file);
    changeWindowTitleIfFocused(bufview);
}

static void handleEvent(Widget *widget, Event event)
{
    BufferView *bufview = (BufferView*) widget;
    GapBuffer *gap = bufview->gap;

    switch (event.type) {

        case EVENT_MOUSE_LEFT_DOWN:
        changeWindowTitle(bufview);
        setFocus(widget);
        manageClick(bufview, event.mouse);
        break;

        case EVENT_MOUSE_LEFT_UP:
        if (bufview->selecting) {
            setMouseFocus(NULL);
            bufview->selecting = false;
        }
        break;

        case EVENT_MOUSE_MOVE:
        if (bufview->selecting) {
            bufview->select_second = getOffsetAssociatedToCoordinates(bufview, event.mouse);
            GapBuffer_moveAbsoluteRaw(gap, bufview->select_second);
        }
        break;

        case EVENT_OPEN: openFile(bufview, event.path); break;
        case EVENT_SAVE: saveFile(bufview); break;

        case EVENT_TEXT:
        removeSelectionAndMoveCursorThere(bufview);
        if (!GapBuffer_insertRune(gap, event.rune)) 
            fprintf(stderr, "Couldn't insert string\n");
        break;

        case EVENT_KEY:
        manageKey(bufview, event.key);
        break;

        default:
        break;
    }
}
