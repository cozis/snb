#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <raylib.h>
#include "dialog.h"
#include "buff_view.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

#define MIN_FONT_SIZE 14
#define MAX_FONT_SIZE 100
#define INC_FONT_SIZE 2

#define SPACES_PER_TAB 4

void manageEvents(GapBuffer *gap, BufferView *bufview, BufferViewStyle *style)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        clickBufferView(bufview, mouse);
    }

    bool control = IsKeyDown(KEY_LEFT_CONTROL) 
                || IsKeyDown(KEY_RIGHT_CONTROL);

    for (int key; (key = GetKeyPressed()) > 0;) {

        switch (key) {

            case KEY_O:
            if (control) {
                char file[1024];
                int num = chooseFile(file, sizeof(file));
                if (num < 0)
                    fprintf(stderr, "Failed to choose file\n");
                else if (num == 0)
                    fprintf(stderr, "Didn't choose a file\n");
                else {
                    GapBuffer_whipeClean(gap);
                    if (!GapBuffer_insertFile(gap, file))
                        fprintf(stderr, "Failed to load '%s'\n", file);
                    else
                        fprintf(stderr, "Loaded '%s'\n", file);
                }
            }
            break;

            case KEY_RIGHT_BRACKET:
            if (control) {
                fprintf(stderr, "Increasing font size\n");
                style->font_size = MIN(style->font_size + INC_FONT_SIZE, MAX_FONT_SIZE);
            }
            break;
            
            case KEY_SLASH:
            if (control) {
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

            char spaces[SPACES_PER_TAB];
            
            case KEY_TAB:
            memset(spaces, ' ', sizeof(spaces));
            if (!GapBuffer_insertString(gap, spaces, sizeof(spaces)))
                fprintf(stderr, "Couldn't insert tab\n");
            break;
        }
    }

    for (int code; (code = GetCharPressed()) > 0;)
        if (!GapBuffer_insertRune(gap, code)) 
            fprintf(stderr, "Couldn't insert string\n");
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
        .line_h   = 1,
        .ruler_x  = 80,
        .cursor_w = 3,
        .color_cursor = RED,
        .color_text   = BLACK,
        .color_ruler  = GRAY,
        .font_path = "SourceCodePro-Regular.ttf",
        .font_size = 24,
    };

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(720, 500, "SNB");

    BufferView buff_view;
    initBufferView(&buff_view, &buff_view_style, gap);

    while (!WindowShouldClose()) {
      
        manageEvents(gap, &buff_view, &buff_view_style);
      
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
