#include <stdio.h>
#include "style.h"
#include "dispatch.h"
#include "spawn_dialog.h"
#include "widget/split_view.h"
#include "utils/get_key_pressed_or_repeated.h"

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

static void saveFileInWidget(Widget *widget)
{
    Event event;
    event.type = EVENT_SAVE;
    event.mouse = GetMousePosition();
    event.mouse.x -= widget->last_offset.x;
    event.mouse.y -= widget->last_offset.y;
    handleWidgetEvent(widget, event);
}

static void insertCharIntoWidget(Widget *widget, int code)
{
    Event event;
    event.type = EVENT_TEXT;
    event.mouse = GetMousePosition();
    event.rune = code;
    event.mouse.x -= widget->last_offset.x;
    event.mouse.y -= widget->last_offset.y;
    handleWidgetEvent(widget, event);
}

static void chooseFileAndOpenIntoWidget(Widget *widget)
{
    char file[1024];
    int num = chooseFileToOpen(file, sizeof(file));
    if (num < 0)
        fprintf(stderr, "Failed to choose file\n");
    else if (num == 0)
        fprintf(stderr, "Didn't choose a file\n");
    else
        openFileIntoWidget(widget, file);
}

static void split(SplitDirection dir)
{
    Widget *focus = getFocus();
    if (focus)
        stylizedSplitView(dir, focus, (Widget*) createStylizedBufferView());
}

static void applyKeyToWidget(Widget *widget, int key)
{
    Event event;
    event.type = EVENT_KEY;
    event.mouse = GetMousePosition();
    event.key = key;
    event.mouse.x -= widget->last_offset.x;
    event.mouse.y -= widget->last_offset.y;
    handleWidgetEvent(widget, event);
}

static void clickOntoWidget(Widget *widget)
{
    Event event;
    event.type = EVENT_MOUSE_LEFT_DOWN;
    event.mouse = GetMousePosition();
    event.mouse.x -= widget->last_offset.x;
    event.mouse.y -= widget->last_offset.y;
    handleWidgetEvent(widget, event);
}

static void unclickFromWidget(Widget *widget)
{
    Event event;
    event.type = EVENT_MOUSE_LEFT_UP;
    event.mouse = GetMousePosition();
    event.mouse.x -= widget->last_offset.x;
    event.mouse.y -= widget->last_offset.y;
    handleWidgetEvent(widget, event);
}

void dispatchEvents(Widget *root)
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

    Vector2 wheel = GetMouseWheelMoveV();
    if (wheel.x != 0 || wheel.y != 0) {
        Event event;
        event.type = EVENT_MOUSE_WHEEL;
        event.mouse = GetMousePosition();
        event.wheel = wheel;
        handleWidgetEvent(root, event);
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