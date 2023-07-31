#include <stddef.h>
#include "widget.h"

void initWidget(Widget *widget, WidgetFuncDraw draw, 
                WidgetFuncFree free, WidgetFuncHandleEvent handleEvent)
{
    widget->parent = NULL;
    widget->last_offset.x = 0;
    widget->last_offset.y = 0;
    widget->last_area.x = 0;
    widget->last_area.y = 0;
    widget->last_logic_area.x = 0;
    widget->last_logic_area.y = 0;
    widget->scroll.x = 0;
    widget->scroll.y = 0;
    widget->scrolling = false;
    widget->target.id = 0;
    widget->draw = draw;
    widget->free = free;
    widget->handleEvent = handleEvent;
}

static Rectangle getVerticalScrollThumbRegion(Widget *widget)
{
    Vector2 scroll = widget->scroll;
    Vector2 area = widget->last_area;
    Vector2 logic_area = widget->last_logic_area;
    float ratio = area.y / logic_area.y;

    Rectangle rect;

    if (logic_area.y > area.y)
        rect.height = area.y * ratio;
    else
        rect.height = 0;

    rect.width = 20;
    rect.x = area.x - rect.width;
    rect.y = scroll.y * ratio;

    return rect;
}

static Rectangle getHorizontalScrollThumbRegion(Widget *widget)
{
    Vector2 scroll = widget->scroll;
    Vector2 area = widget->last_area;
    Vector2 logic_area = widget->last_logic_area;
    float ratio = area.x / logic_area.x;

    Rectangle rect;

    if (logic_area.x > area.x)
        rect.width = area.x * ratio;
    else
        rect.width = 0;

    rect.height = 20;
    rect.x = scroll.x * ratio;
    rect.y = area.y - rect.height;

    return rect;
}

bool isBiggerThanViewport(Widget *widget)
{
    return widget->last_logic_area.x > widget->last_area.x
        || widget->last_logic_area.y > widget->last_area.y;
}

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

float clampVerticalScrollValue(Widget *widget, float value)
{
    float min_scroll = 0;
    float max_scroll = widget->last_logic_area.y - widget->last_area.y;
    value = MIN(max_scroll, value);
    value = MAX(min_scroll, value);
    return value;
}

float clampHorizontalScrollValue(Widget *widget, float value)
{
    float min_scroll = 0;
    float max_scroll = widget->last_logic_area.x - widget->last_area.x;
    value = MIN(max_scroll, value);
    value = MAX(min_scroll, value);
    return value;
}

void drawWidget(Widget *widget, Vector2 offset, Vector2 area)
{
    Vector2 logic_area;
    if (isBiggerThanViewport(widget)) {
        
        Vector2 logic_offset;
        logic_offset.x = - widget->scroll.x;
        logic_offset.y = - widget->scroll.y;

        if (widget->target.texture.width != area.x || widget->target.texture.height != area.y) {
            UnloadRenderTexture(widget->target);
            widget->target.id = 0;
        }

        if (widget->target.id == 0)
            widget->target = LoadRenderTexture(area.x, area.y);

        BeginTextureMode(widget->target);
        DrawRectangle(0, 0, area.x, area.y, WHITE);
        logic_area = widget->draw(widget, logic_offset, area);
        DrawRectangleRec(getVerticalScrollThumbRegion(widget), RED);
        DrawRectangleRec(getHorizontalScrollThumbRegion(widget), RED);
        EndTextureMode();

        Rectangle src, dst;
        src.x = 0;
        src.y = 0;
        src.width  =  area.x;
        src.height = -area.y;
        dst.x = offset.x;
        dst.y = offset.y;
        dst.width  = area.x;
        dst.height = area.y;
        Vector2 org = {0, 0};
        DrawTexturePro(widget->target.texture, src, dst, org, 0, WHITE);

    } else {
        
        DrawRectangle(offset.x, offset.y, area.x, area.y, WHITE);

        Vector2 logic_offset;
        logic_offset.x = offset.x - widget->scroll.x;
        logic_offset.y = offset.y - widget->scroll.y;

        logic_area = widget->draw(widget, logic_offset, area);
    }

    widget->last_offset = offset;
    widget->last_area = area;
    widget->last_logic_area = logic_area;

    widget->scroll.x = clampHorizontalScrollValue(widget, widget->scroll.x);
    widget->scroll.y =   clampVerticalScrollValue(widget, widget->scroll.y);
}

#include <stdio.h>
void handleWidgetEvent(Widget *widget, Event event)
{
    bool handled = false;

    switch (event.type) {

        case EVENT_MOUSE_MOVE:
        if (widget->scrolling) {

            if (widget->scrolldir) {

                // Scrolling vertically
                float ratio = widget->last_logic_area.y / widget->last_area.y;
                float delta = (event.mouse.y - widget->mouse_start) * ratio;
                widget->scroll.y = widget->scroll_start + delta;
                widget->scroll.y = clampVerticalScrollValue(widget, widget->scroll.y);

            } else {

                // Scrolling horizontally
                float ratio = widget->last_logic_area.x / widget->last_area.x;
                float delta = (event.mouse.x - widget->mouse_start) * ratio;
                widget->scroll.x = widget->scroll_start + delta;
                widget->scroll.x = clampHorizontalScrollValue(widget, widget->scroll.x);
            }
            handled = true;
        }
        break;
        
        case EVENT_MOUSE_LEFT_UP:
        if (widget->scrolling) {
            handled = true;
            widget->scrolling = false;
            setMouseFocus(NULL);
        }
        break;
        
        case EVENT_MOUSE_LEFT_DOWN:
        if (CheckCollisionPointRec(event.mouse, getVerticalScrollThumbRegion(widget))) {
            handled = true;
            widget->scrolling = true;
            widget->scrolldir = true;
            widget->mouse_start = event.mouse.y;
            widget->scroll_start = widget->scroll.y;
            setMouseFocus(widget);
        } else if (CheckCollisionPointRec(event.mouse, getHorizontalScrollThumbRegion(widget))) {
            handled = true;
            widget->scrolling = true;
            widget->scrolldir = false;
            widget->mouse_start = event.mouse.x;
            widget->scroll_start = widget->scroll.x;
            setMouseFocus(widget);
        }
        break;
    }

    if (!handled) {
        event.mouse.x += widget->scroll.x;
        event.mouse.y += widget->scroll.y;
        widget->handleEvent(widget, event);
    }
}

static Widget *focus = NULL;
static Widget *mouse_focus = NULL;

void freeWidget(Widget *widget)
{
    if (widget->target.id != 0)
        UnloadRenderTexture(widget->target);
    
    if (widget->free)
        widget->free(widget);
    
    if (focus == widget)
        focus = NULL;
}

void setFocus(Widget *widget)
{
    focus = widget;
}

Widget *getFocus(void)
{
    return focus;
}

void setMouseFocus(Widget *widget)
{
    mouse_focus = widget;
}

Widget *getMouseFocus(void)
{
    return mouse_focus;
}
