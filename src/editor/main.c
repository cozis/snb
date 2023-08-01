#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <raylib.h>
#include "event.h"
#include "dialog.h"
#include "buff_view.h"
#include "split_view.h"
#include "get_key_pressed_or_repeated.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

#define MIN_FONT_SIZE 14
#define MAX_FONT_SIZE 100
#define INC_FONT_SIZE 2

BufferViewStyle style;

void openFileIntoWidget(Widget *widget, const char *file)
{
    Event event;
    event.type = EVENT_OPEN;
    event.mouse = GetMousePosition();
    event.path = file;
    event.mouse.x -= widget->last_offset.x;
    event.mouse.y -= widget->last_offset.y;
    handleWidgetEvent(widget, event);
}

void saveFileInWidget(Widget *widget)
{
    Event event;
    event.type = EVENT_SAVE;
    event.mouse = GetMousePosition();
    event.mouse.x -= widget->last_offset.x;
    event.mouse.y -= widget->last_offset.y;
    handleWidgetEvent(widget, event);
}

void insertCharIntoWidget(Widget *widget, int code)
{
    Event event;
    event.type = EVENT_TEXT;
    event.mouse = GetMousePosition();
    event.rune = code;
    event.mouse.x -= widget->last_offset.x;
    event.mouse.y -= widget->last_offset.y;
    handleWidgetEvent(widget, event);
}

void chooseFileAndOpenIntoWidget(Widget *widget)
{
    char file[1024];
    int num = chooseFile(file, sizeof(file));
    if (num < 0)
        fprintf(stderr, "Failed to choose file\n");
    else if (num == 0)
        fprintf(stderr, "Didn't choose a file\n");
    else
        openFileIntoWidget(widget, file);
}

void split(SplitDirection dir)
{
    Widget *focus = getFocus();
    if (!focus)
        return;

    Widget *new_widget = createBufferView(&style);
    if (!new_widget)
        return;

    if (!splitView(dir, focus, new_widget))
        freeWidget(new_widget);
}

void applyKeyToWidget(Widget *widget, int key)
{
    Event event;
    event.type = EVENT_KEY;
    event.mouse = GetMousePosition();
    event.key = key;
    event.mouse.x -= widget->last_offset.x;
    event.mouse.y -= widget->last_offset.y;
    handleWidgetEvent(widget, event);
}

void clickOntoWidget(Widget *widget)
{
    Event event;
    event.type = EVENT_MOUSE_LEFT_DOWN;
    event.mouse = GetMousePosition();
    event.mouse.x -= widget->last_offset.x;
    event.mouse.y -= widget->last_offset.y;
    handleWidgetEvent(widget, event);
}

void unclickFromWidget(Widget *widget)
{
    Event event;
    event.type = EVENT_MOUSE_LEFT_UP;
    event.mouse = GetMousePosition();
    event.mouse.x -= widget->last_offset.x;
    event.mouse.y -= widget->last_offset.y;
    handleWidgetEvent(widget, event);
}

void increaseFontSize(void)
{
    style.font_size = MIN(style.font_size + INC_FONT_SIZE, MAX_FONT_SIZE);
}

void decreaseFontSize(void)
{
    style.font_size = MAX(style.font_size - INC_FONT_SIZE, MIN_FONT_SIZE);
}

void manageEvents(Widget *root)
{
    Widget *focus = getFocus();
    Widget *mouse_focus = getMouseFocus();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        clickOntoWidget(root);

    if (mouse_focus && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        unclickFromWidget(mouse_focus);

    if (mouse_focus) {
        Event event;
        event.type = EVENT_MOUSE_MOVE;
        event.mouse = GetMousePosition();
        event.mouse.x -= mouse_focus->last_offset.x;
        event.mouse.y -= mouse_focus->last_offset.y;
        handleWidgetEvent(mouse_focus, event);
    }
    
    int first_repeat_freq = 500000;
    int       repeat_freq =  30000;
    for (int key, repeat; (key = GetKeyPressedOrRepeated(&repeat, repeat_freq, first_repeat_freq)) > 0;) {
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
            if (!repeat) {
                switch (key) {
                    case KEY_UP:    split(SPLIT_UP);    break;
                    case KEY_DOWN:  split(SPLIT_DOWN);  break;
                    case KEY_LEFT:  split(SPLIT_LEFT);  break;
                    case KEY_RIGHT: split(SPLIT_RIGHT); break;
                    case KEY_O: if (focus) chooseFileAndOpenIntoWidget(focus); break;
                    case KEY_S: if (focus) saveFileInWidget(focus); break;
                    case KEY_RIGHT_BRACKET: increaseFontSize(); break;
                    case KEY_SLASH:         decreaseFontSize();break;
                }
            }
        } else {
            if (focus)
                applyKeyToWidget(focus, key);
        }
    }

    for (int code; (code = GetCharPressed()) > 0;)
        if (focus) insertCharIntoWidget(focus, code);
}

int main(int argc, char **argv)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(720, 500, "SNB");
    
    style = (BufferViewStyle) {
        .line_h   = 1,
        .ruler_x  = 80,
        .cursor_w = 3,
        .pad_h = 10,
        .pad_v = 10,
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

    Widget *root = createBufferView(&style);
    if (root == NULL)
        return -1;
    root->parent = &root;

    if (file)
        openFileIntoWidget(root, file);

    while (!WindowShouldClose()) {
        manageEvents(root);
        BeginDrawing();
        ClearBackground(WHITE);
        Vector2 offset = {0, 0};
        Vector2 area = {GetScreenWidth(), GetScreenHeight()};
        drawWidget(root, offset, area);
        EndDrawing();
    }
    freeWidget(root);
    CloseWindow();
    return 0;
}
