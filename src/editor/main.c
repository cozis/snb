#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <raylib.h>
#include "event.h"
#include "dialog.h"
#include "buff_view.h"
#include "split_view.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

#define MIN_FONT_SIZE 14
#define MAX_FONT_SIZE 100
#define INC_FONT_SIZE 2

void manageEvents(Widget *root, BufferViewStyle *style)
{
    Vector2 mouse = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Event event;
        event.type = EVENT_MOUSE_LEFT_DOWN;
        event.mouse = mouse;
        handleWidgetEvent(root, event);
    }

    bool control = IsKeyDown(KEY_LEFT_CONTROL) 
                || IsKeyDown(KEY_RIGHT_CONTROL);

    for (int key; (key = GetKeyPressed()) > 0;) {
        Widget *focus = getFocus();
        if (control) {
            switch (key) {

                case KEY_UP:
                {
                    if (!focus)
                        break;
                    Widget *new_widget = createBufferView(style);
                    if (!new_widget)
                        break;
                    if (!splitView(SPLIT_UP, focus, new_widget))
                        freeWidget(new_widget);
                }
                break;

                case KEY_DOWN:
                {
                    if (!focus)
                        break;
                    Widget *new_widget = createBufferView(style);
                    if (!new_widget)
                        break;
                    if (!splitView(SPLIT_DOWN, focus, new_widget))
                        freeWidget(new_widget);
                }
                break;

                case KEY_LEFT:
                {
                    if (!focus)
                        break;
                    Widget *new_widget = createBufferView(style);
                    if (!new_widget)
                        break;
                    if (!splitView(SPLIT_LEFT, focus, new_widget))
                        freeWidget(new_widget);
                }
                break;

                case KEY_RIGHT:
                {
                    if (!focus)
                        break;
                    Widget *new_widget = createBufferView(style);
                    if (!new_widget)
                        break;
                    if (!splitView(SPLIT_RIGHT, focus, new_widget))
                        freeWidget(new_widget);
                }
                break;

                case KEY_O:
                if (focus) {
                    char file[1024];
                    int num = chooseFile(file, sizeof(file));
                    if (num < 0)
                        fprintf(stderr, "Failed to choose file\n");
                    else if (num == 0)
                        fprintf(stderr, "Didn't choose a file\n");
                    else {
                        Event event;
                        event.type = EVENT_OPEN;
                        event.mouse = mouse;
                        event.path = file;
                        handleWidgetEvent(focus, event);
                    }
                }
                break;

                case KEY_RIGHT_BRACKET:
                fprintf(stderr, "Increasing font size\n");
                style->font_size = MIN(style->font_size + INC_FONT_SIZE, MAX_FONT_SIZE);
                break;
                
                case KEY_SLASH:
                fprintf(stderr, "Decreasing font size\n");
                style->font_size = MAX(style->font_size - INC_FONT_SIZE, MIN_FONT_SIZE);
                break;
            }
        } else {
            Widget *focus = getFocus();
            if (focus) {
                Event event;
                event.type = EVENT_KEY;
                event.mouse = mouse;
                event.key = key;
                handleWidgetEvent(focus, event);
            }
        }
    }

    for (int code; (code = GetCharPressed()) > 0;) {
        Widget *focus = getFocus();
        if (focus) {
            Event event;
            event.type = EVENT_TEXT;
            event.mouse = mouse;
            event.rune = code;
            handleWidgetEvent(focus, event);
        }
    }
}

int main(int argc, char **argv)
{
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

    const char *file;
    if (argc > 1)
        file = argv[1];
    else
        file = NULL;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(720, 500, "SNB");

    Widget *root = createBufferView(&buff_view_style);
    if (root == NULL)
        return -1;
    root->parent = &root;

    if (file) {
        Event event;
        event.type = EVENT_OPEN;
        event.mouse = (Vector2) {0, 0};
        event.path = file;
        handleWidgetEvent(root, event);
    }

    while (!WindowShouldClose()) {
        manageEvents(root, &buff_view_style);
        BeginDrawing();
        ClearBackground(WHITE);
        Vector2 offset = {0, 0};
        Vector2 area = {GetScreenWidth(), GetScreenHeight()};
        drawWidget(root, offset, area);
        EndDrawing();
    }

    CloseWindow();
    freeWidget(root);
    return 0;
}
