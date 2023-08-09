#include <math.h> // fabs
#include <assert.h>
#include <stddef.h>
#include "../utils/basic.h"
#include "widget.h"

void initWidget(Widget *widget, WidgetStyle *style, WidgetFuncDraw draw, 
                WidgetFuncFree free, WidgetFuncHandleEvent handleEvent)
{
    widget->style = style;
    widget->parent = NULL;
    widget->last_offset.x = 0;
    widget->last_offset.y = 0;
    widget->last_area.x = 0;
    widget->last_area.y = 0;
    widget->last_logic_area.x = 0;
    widget->last_logic_area.y = 0;
    widget->desired_area.x = -1;
    widget->desired_area.y = -1;
    widget->margin.x = 0;
    widget->margin.y = 0;
    widget->scroll.x = 0;
    widget->scroll.y = 0;
    widget->scrolling = false;
    widget->target.id = 0;
    widget->draw = draw;
    widget->free = free;
    widget->handleEvent = handleEvent;
}

void setMarginX(Widget *widget, float x)
{
    widget->margin.x = x;
}

void setMarginY(Widget *widget, float y)
{
    widget->margin.y = y;
}

float getDesiredWidth(Widget *widget)
{
    float desired_w;
    if (widget->desired_area.x < 0)
        desired_w = widget->last_logic_area.x;
    else
        desired_w = widget->desired_area.x;
    return desired_w;
}

float getDesiredHeight(Widget *widget)
{
    float desired_h;
    if (widget->desired_area.y < 0)
        desired_h = widget->last_logic_area.y;
    else
        desired_h = widget->desired_area.y;
    return desired_h;
}

Vector2 getDesiredArea(Widget *widget) 
{
    return (Vector2) {getDesiredWidth(widget), getDesiredHeight(widget)};
}

void setDesiredWidth(Widget *widget, float desired_w)
{
    assert(desired_w >= 0);
    widget->desired_area.x = desired_w;
}

void setDesiredHeight(Widget *widget, float desired_h)
{
    assert(desired_h >= 0);
    widget->desired_area.y = desired_h;
}

void removeDesiredWidth(Widget *widget)
{
    widget->desired_area.x = -1;
}

void removeDesiredHeight(Widget *widget)
{
    widget->desired_area.y = -1;
}

Vector2 getLastDrawArea(Widget *widget)
{
    return widget->last_area;
}

static bool horizontallyBiggerThanViewport(Widget *widget)
{
    return widget->last_area.x < widget->last_logic_area.x;
}

static bool verticallyBiggerThanViewport(Widget *widget)
{
    return widget->last_area.y < widget->last_logic_area.y;
}

static bool horizontalScrollbarShown(Widget *widget)
{
    return horizontallyBiggerThanViewport(widget) && widget->style->show_scrollbar_h;
}

static bool verticalScrollbarShown(Widget *widget)
{
    return verticallyBiggerThanViewport(widget) && widget->style->show_scrollbar_v;
}

Vector2 getLastLogicDrawArea(Widget *widget)
{
    float thumb_margin = widget->style->scrollbar_thumb_margin;
    float thumb_width  = widget->style->scrollbar_thumb_width;

    Vector2 logic_area = widget->last_logic_area;
    
    if (horizontalScrollbarShown(widget)) logic_area.y += thumb_width + 2 * thumb_margin;
    if (verticalScrollbarShown(widget)) logic_area.x += thumb_width + 2 * thumb_margin;
    
    return logic_area;
}

Vector2 getScroll(Widget *widget)
{
    return widget->scroll;
}

static bool isBiggerThanViewport(Widget *widget)
{
    return horizontallyBiggerThanViewport(widget)
        ||   verticallyBiggerThanViewport(widget);
}

static Rectangle getVerticalScrollThumbRegion(Widget *widget)
{
    Vector2     scroll = getScroll(widget);
    Vector2       area = getLastDrawArea(widget);
    Vector2 logic_area = getLastLogicDrawArea(widget);

    float thumb_margin = widget->style->scrollbar_thumb_margin;
    float thumb_width  = widget->style->scrollbar_thumb_width;

    float track_h = area.y - 2 * thumb_margin;
    if (horizontalScrollbarShown(widget))
        track_h -= thumb_width + 2 * thumb_margin;

    Rectangle rect;

    if (logic_area.y > area.y)
        rect.height = track_h * area.y / logic_area.y;
    else
        rect.height = 0;

    rect.width = thumb_width;
    rect.x = area.x - rect.width - thumb_margin;
    rect.y = scroll.y * track_h / logic_area.y + thumb_margin;

    return rect;
}

static Rectangle getHorizontalScrollThumbRegion(Widget *widget)
{
    Vector2 scroll = getScroll(widget);
    Vector2 area = getLastDrawArea(widget);
    Vector2 logic_area = getLastLogicDrawArea(widget);

    float thumb_margin = widget->style->scrollbar_thumb_margin;
    float thumb_width  = widget->style->scrollbar_thumb_width;

    float track_w = area.x - 2 * thumb_margin;
    if (verticalScrollbarShown(widget))
        track_w -= thumb_width + 2 * thumb_margin;

    Rectangle rect;

    if (logic_area.x > area.x)
        rect.width = track_w * area.x / logic_area.x;
    else
        rect.width = 0;

    rect.height = thumb_width;
    rect.x = scroll.x * track_w / logic_area.x + thumb_margin;
    rect.y = area.y - rect.height - thumb_margin;

    return rect;
}

static Rectangle getHorizontalScrollTrackRegion(Widget *widget)
{
    Vector2 area = getLastDrawArea(widget);

    float thumb_margin = widget->style->scrollbar_thumb_margin;
    float thumb_width  = widget->style->scrollbar_thumb_width;

    Rectangle rect;

    if (horizontalScrollbarShown(widget)) {
        rect.width = area.x - 2 * thumb_margin;
        if (verticalScrollbarShown(widget))
            rect.width -= thumb_width + 2 * thumb_margin;
    } else
        rect.width = 0;

    rect.height = thumb_width;
    rect.x = thumb_margin;
    rect.y = area.y - rect.height - thumb_margin;

    return rect;
}

static Rectangle getVerticalScrollTrackRegion(Widget *widget)
{
    Vector2 area = getLastDrawArea(widget);

    float thumb_margin = widget->style->scrollbar_thumb_margin;
    float thumb_width  = widget->style->scrollbar_thumb_width;

    Rectangle rect;

    if (verticalScrollbarShown(widget)) {
        rect.height = area.y - 2 * thumb_margin;
        if (horizontalScrollbarShown(widget))
            rect.height -= thumb_width + 2 * thumb_margin;
    } else
        rect.height = 0;

    rect.width = thumb_width;
    rect.x = area.x - rect.width - thumb_margin;
    rect.y = thumb_margin;

    return rect;
}

static Rectangle getVerticalScrollbarRegion(Widget *widget)
{
    float thumb_margin = widget->style->scrollbar_thumb_margin;

    Rectangle rect = getVerticalScrollTrackRegion(widget);
    rect.x -= thumb_margin;
    rect.y -= thumb_margin;
    rect.width  += 2 * thumb_margin;
    rect.height += 2 * thumb_margin;

    return rect;
}

static Rectangle getHorizontalScrollbarRegion(Widget *widget)
{
    float thumb_margin = widget->style->scrollbar_thumb_margin;

    Rectangle rect = getHorizontalScrollTrackRegion(widget);
    rect.x -= thumb_margin;
    rect.y -= thumb_margin;
    rect.width  += 2 * thumb_margin;
    rect.height += 2 * thumb_margin;

    return rect;
}

static float clampVerticalScrollValue(Widget *widget, float value)
{
    float min_scroll = 0;
    float max_scroll = widget->last_logic_area.y - widget->last_area.y;
    if (horizontalScrollbarShown(widget))
        max_scroll += getHorizontalScrollbarRegion(widget).height;

    value = MIN(max_scroll, value);
    value = MAX(min_scroll, value);
    return value;
}

static float clampHorizontalScrollValue(Widget *widget, float value)
{
    float min_scroll = 0;
    float max_scroll = widget->last_logic_area.x - widget->last_area.x;
    if (verticalScrollbarShown(widget))
        max_scroll += getVerticalScrollbarRegion(widget).width;
    value = MIN(max_scroll, value);
    value = MAX(min_scroll, value);
    return value;
}

static void drawScrollbars(Widget *widget)
{
    float       roundness = widget->style->scrollbar_thumb_roundness;
    int          segments = widget->style->scrollbar_thumb_segments;
    Color     track_color = widget->style->scrollbar_track_color;
    Color     thumb_color = widget->style->scrollbar_thumb_color;
    Color    active_color = widget->style->scrollbar_thumb_active_color;
    Color scrollbar_color = widget->style->scrollbar_color;

    Color v_thumb_color;
    Color h_thumb_color;
    if (widget->scrolling) {
        if (widget->scrolldir) {
            // Scrolling vertically
            h_thumb_color =  thumb_color;
            v_thumb_color = active_color;
        } else {
            // Scrolling horizontally
            h_thumb_color = active_color;
            v_thumb_color =  thumb_color;
        }
    } else {
        // Not scrolling
        h_thumb_color = thumb_color;
        v_thumb_color = thumb_color;
    }

    Rectangle v_region = getVerticalScrollbarRegion(widget);
    Rectangle h_region = getHorizontalScrollbarRegion(widget);

    if (verticalScrollbarShown(widget)) {
        DrawRectangleRec(v_region, scrollbar_color);
        DrawRectangleRounded(getVerticalScrollTrackRegion(widget), roundness, segments, track_color);
        DrawRectangleRounded(getVerticalScrollThumbRegion(widget), roundness, segments, v_thumb_color);
    }

    if (horizontalScrollbarShown(widget)) {
        DrawRectangleRec(h_region, scrollbar_color);
        DrawRectangleRounded(getHorizontalScrollTrackRegion(widget), roundness, segments, track_color);
        DrawRectangleRounded(getHorizontalScrollThumbRegion(widget), roundness, segments, h_thumb_color);
    }

    if (horizontalScrollbarShown(widget) && verticalScrollbarShown(widget)) {
        Rectangle box;
        box.x = h_region.x + h_region.width;
        box.y = v_region.y + v_region.height;
        box.width  = v_region.width;
        box.height = h_region.height;
        DrawRectangleRec(box, scrollbar_color);
    }
}

static void
drawBackground(Widget *widget, Vector2 offset, Vector2 area)
{
    float roundness = widget->style->roundness;
    int    segments = widget->style->segments;
    Color     color = widget->style->color_background;

    Rectangle rect;
    rect.x = offset.x;
    rect.y = offset.y;
    rect.width  = area.x;
    rect.height = area.y;

    DrawRectangleRounded(rect, roundness, segments, color);
}

void drawWidget(Widget *widget, Vector2 offset, Vector2 area)
{
    Vector2 logic_area;
    if (isBiggerThanViewport(widget)) {
        
        Vector2 logic_offset;
        logic_offset.x = - widget->scroll.x;
        logic_offset.y = - widget->scroll.y;

        bool area_bigger_than_texture = fabs(widget->target.texture.width  - area.x) >= 1 
                                     || fabs(widget->target.texture.height - area.y) >= 1;
        if (area_bigger_than_texture) {
            UnloadRenderTexture(widget->target);
            widget->target.id = 0;
        }

        int w = ceil(area.x);
        int h = ceil(area.y);
        if (widget->target.id == 0 && w > 0 && h > 0)
            widget->target = LoadRenderTexture(w, h);

        BeginTextureMode(widget->target);
        drawBackground(widget, (Vector2) {.x=0, .y=0}, area);
        logic_area = widget->draw(widget, logic_offset, area);
        drawScrollbars(widget);
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
        
        drawBackground(widget, offset, area);
        
        Vector2 logic_offset;
        logic_offset.x = offset.x - widget->scroll.x;
        logic_offset.y = offset.y - widget->scroll.y;

        logic_area = widget->draw(widget, logic_offset, area);
    }
/*
    {
        Rectangle rect;
        rect.x = offset.x;
        rect.y = offset.y;
        rect.width = area.x;
        rect.height = area.y;
        DrawRectangleLinesEx(rect, 1, RED);
    }
*/
    widget->last_offset = offset;
    widget->last_area = area;
    widget->last_logic_area = logic_area;

    widget->scroll.x = clampHorizontalScrollValue(widget, widget->scroll.x);
    widget->scroll.y =   clampVerticalScrollValue(widget, widget->scroll.y);
}

void handleWidgetEvent(Widget *widget, Event event)
{
    bool handled = false;

    switch (event.type) {

        case EVENT_MOUSE_WHEEL:
        if (!widget->scrolling) {
            Vector2 scale = {.x=50, .y=50};
            Vector2 delta = event.wheel;
            widget->scroll.x -= scale.x * delta.x;
            widget->scroll.y -= scale.y * delta.y;
            widget->scroll.x = clampHorizontalScrollValue(widget, widget->scroll.x);
            widget->scroll.y = clampVerticalScrollValue(widget, widget->scroll.y);
        }
        break;

        case EVENT_MOUSE_MOVE:
        if (widget->scrolling) {

            Vector2 logic_area = getLastLogicDrawArea(widget);

            if (widget->scrolldir) {
                
                // Scrolling vertically
                float track = getVerticalScrollTrackRegion(widget).height;
                float delta = (event.mouse.y - widget->mouse_start) * logic_area.y / track;
                widget->scroll.y = widget->scroll_start + delta;
                widget->scroll.y = clampVerticalScrollValue(widget, widget->scroll.y);

            } else {

                // Scrolling horizontally
                float track = getHorizontalScrollTrackRegion(widget).width;
                float delta = (event.mouse.x - widget->mouse_start) * logic_area.x / track;
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

        default:
        break;
    }

    if (!handled) {
        event.mouse.x += widget->scroll.x;
        event.mouse.y += widget->scroll.y;
        widget->handleEvent(widget, event);
    }
}

void setScrollX(Widget *widget, float x)
{
    widget->scroll.x = clampHorizontalScrollValue(widget, x);
}

void setScrollY(Widget *widget, float y)
{
    widget->scroll.y = clampVerticalScrollValue(widget, y);
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
    if (mouse_focus == widget)
        mouse_focus = NULL;
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
