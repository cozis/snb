#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <raylib.h>
#include "buff_view.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

#define MIN_FONT_SIZE 14
#define MAX_FONT_SIZE 100
#define INC_FONT_SIZE 2

void manageEvents(GapBuffer *gap, BufferViewStyle *style)
{
    bool control_pressed = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    for (int key; (key = GetKeyPressed()) > 0;) {
        
        switch (key) {

            case KEY_RIGHT_BRACKET:
            if (control_pressed) {
                fprintf(stderr, "Increasing font size\n");
                style->font_size = MIN(style->font_size + INC_FONT_SIZE, MAX_FONT_SIZE);
            }
            break;
            
            case KEY_SLASH:
            if (control_pressed) {
                fprintf(stderr, "Decreasing font size\n");
                style->font_size = MAX(style->font_size - INC_FONT_SIZE, MIN_FONT_SIZE);
            }
            break;

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

        bool ok = GapBuffer_insertRune(gap, code);

        if (!ok)
            fprintf(stderr, "Couldn't insert string\n");
    }
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
        if (!GapBuffer_insertFile(gap, file)) {
            fprintf(stderr, "Failed to load '%s'\n", file);
            return -1;
        }
        fprintf(stderr, "Loaded '%s'\n", file);
    }

    BufferViewStyle buff_view_style = {
        .line_h = 1,
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
      
        manageEvents(gap, &buff_view_style);
      
        BeginDrawing();
        ClearBackground(WHITE);
        drawBufferView(&buff_view);
        drawRuler(font, 80, GRAY);
        EndDrawing();
    }

    CloseWindow();
    freeBufferView(&buff_view);
    GapBuffer_destroy(gap);
    return 0;
}
