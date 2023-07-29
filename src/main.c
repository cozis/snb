#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <raylib.h>
#include "gap_buffer.h"

// https://stackoverflow.com/questions/42012563/convert-unicode-code-points-to-utf-8-and-utf-32
static size_t codeToUTF8(unsigned char *const buffer, const unsigned int code)
{
    if (code <= 0x7F) {
        buffer[0] = code;
        return 1;
    }
    if (code <= 0x7FF) {
        buffer[0] = 0xC0 | (code >> 6);            /* 110xxxxx */
        buffer[1] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
        return 2;
    }
    if (code <= 0xFFFF) {
        buffer[0] = 0xE0 | (code >> 12);           /* 1110xxxx */
        buffer[1] = 0x80 | ((code >> 6) & 0x3F);   /* 10xxxxxx */
        buffer[2] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
        return 3;
    }
    if (code <= 0x10FFFF) {
        buffer[0] = 0xF0 | (code >> 18);           /* 11110xxx */
        buffer[1] = 0x80 | ((code >> 12) & 0x3F);  /* 10xxxxxx */
        buffer[2] = 0x80 | ((code >> 6) & 0x3F);   /* 10xxxxxx */
        buffer[3] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
        return 4;
    }
    return 0;
}

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
calculateStringRenderWidth(Font font, int font_size,
                           const char *str, size_t len)
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

static void
drawBufferContents(GapBuffer *gap, Font font,
                   size_t cursor)
{
    float font_size = 24;
    float line_h = font_size+4;
    float cursor_w = 3;
    Color cursor_color = RED;
    Color   font_color = BLACK;

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
            int relative_cursor_x = calculateStringRenderWidth(font, font_size, line.str, cursor - line_offset);
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

void manageEvents(GapBuffer *gap, size_t *cursor)
{
    for (int key; (key = GetKeyPressed()) > 0;) {
        switch (key) {

            case KEY_LEFT:
            *cursor = GapBuffer_moveRelative(gap, -1);
            break;

            case KEY_RIGHT:
            *cursor = GapBuffer_moveRelative(gap, 1);
            break;

            case KEY_ENTER:
            if (GapBuffer_insertString(gap, "\n", 1))
                (*cursor)++;
            else
                fprintf(stderr, "Couldn't insert string\n");
            break;

            case KEY_BACKSPACE:
            if (*cursor > 0)
                *cursor -= GapBuffer_removeBackwards(gap, 1);
            break;
        }
    }

    for (int code; (code = GetCharPressed()) > 0;) {

        char temp[4];
        size_t num;

        num = codeToUTF8((unsigned char*) temp, code);
        bool ok = GapBuffer_insertString(gap, temp, num);
        if (ok)
            *cursor += num;
        else
            fprintf(stderr, "Couldn't insert string\n");
    }
}

bool load_file(const char *file, GapBuffer *gap)
{
    FILE *stream = fopen(file, "rb");
    if (stream == NULL)
        return false;

    char buffer[1024];
    for (bool done = false; !done;) {
        size_t num = fread(buffer, sizeof(buffer[0]), sizeof(buffer), stream);
        if (num < sizeof(buffer)) {
            if (ferror(stream))
                goto ouch; // Failed to read from stream
            done = true;
        }
        bool ok = GapBuffer_insertString(gap, buffer, num);
        if (!ok)
            // NOTE: It's possible to have a failure because
            //       the buffer doesn't contain valid utf-8
            //       when a multi-byte symbol is truncated
            //       while reading into the fixed-size buffer.
            goto ouch; // File too big or invalid utf-8
    }
    GapBuffer_moveAbsolute(gap, 0);

    fclose(stream);
    return true;

ouch:
    fclose(stream);
    return false;
}

void drawRuler(Font font, int ruler_width, Color color) {
    int font_size = 24;
    float font_width = MeasureTextEx(font, "A", font_size, 0).x;
    int x = ruler_width * font_width;
    DrawLine(x, 0, x, GetScreenHeight(), color);
}

int main(int argc, char **argv)
{
    char mem[1 << 16];
    GapBuffer *gap = GapBuffer_createUsingMemory(mem, sizeof(mem), NULL);
    if (gap == NULL)
        return -1;

    if (argc > 1) {
        const char *file = argv[1];
        if (!load_file(file, gap)) {
            fprintf(stderr, "Failed to load '%s'\n", file);
            return -1;
        }
        fprintf(stderr, "Loaded '%s'\n", file);
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(720, 500, "SNB");

    Font font = LoadFont("SourceCodePro-Regular.ttf");

    size_t cursor = 0;
    while (!WindowShouldClose()) {
        manageEvents(gap, &cursor);
        BeginDrawing();
        ClearBackground(WHITE);
        drawBufferContents(gap, font, cursor);
        drawRuler(font, 80, GRAY);
        EndDrawing();
    }

    CloseWindow();
    GapBuffer_destroy(gap);
    return 0;
}
