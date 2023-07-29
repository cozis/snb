#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <raylib.h>
#include "buff_view.h"

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

void manageEvents(GapBuffer *gap)
{
    for (int key; (key = GetKeyPressed()) > 0;) {
        switch (key) {

            case KEY_LEFT:  GapBuffer_moveRelative(gap, -1); break;
            case KEY_RIGHT: GapBuffer_moveRelative(gap, +1); break;
            
            case KEY_ENTER:
            if (!GapBuffer_insertString(gap, "\n", 1))
                fprintf(stderr, "Couldn't insert string\n");
            break;
            
            case KEY_BACKSPACE: 
            GapBuffer_removeBackwards(gap, 1);
            break;
        }
    }

    for (int code; (code = GetCharPressed()) > 0;) {

        char temp[4];
        size_t num;

        num = codeToUTF8((unsigned char*) temp, code);
        bool ok = GapBuffer_insertString(gap, temp, num);
        if (!ok)
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

    BufferViewStyle buff_view_style = {
        .line_h = 20,
        .cursor_w = 3,
        .color_cursor = RED,
        .color_text = BLACK,
        .font_path = "SourceCodePro-Regular.ttf",
        .font_size = 24,
    };

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(720, 500, "SNB");

    BufferView buff_view;
    initBufferView(&buff_view, &buff_view_style, gap);

    while (!WindowShouldClose()) {
        manageEvents(gap);
        BeginDrawing();
        ClearBackground(WHITE);
        drawBufferView(&buff_view);
        EndDrawing();
    }

    CloseWindow();
    freeBufferView(&buff_view);
    GapBuffer_destroy(gap);
    return 0;
}