#ifndef SNB_WIDGET_H
#define SNB_WIDGET_H

#include <raylib.h>
#include "event.h"

typedef struct Widget Widget;

typedef Vector2 (*WidgetFuncDraw)(Widget *widget, Vector2 offset, Vector2 area);
typedef void    (*WidgetFuncFree)(Widget *widget);
typedef void    (*WidgetFuncHandleEvent)(Widget *widget, Event event);

typedef struct {
    bool show_scrollbar_h;
    bool show_scrollbar_v;
    Color color_background;
    float scrollbar_thumb_roundness;
    int   scrollbar_thumb_segments;
    float scrollbar_thumb_width;
    float scrollbar_thumb_margin;
    Color scrollbar_color;
    Color scrollbar_track_color;
    Color scrollbar_thumb_color;
    Color scrollbar_thumb_active_color;
    float roundness;
    int   segments;
} WidgetStyle;

struct Widget {
    WidgetStyle *style;
    Widget **parent;
    Vector2 desired_area;
    Vector2 last_offset;
    Vector2 last_area;
    Vector2 last_logic_area;
    Vector2 scroll;
    Vector2 margin;
    bool scrolling;
    bool scrolldir;
    float  mouse_start;
    float scroll_start;
    RenderTexture2D target;
    WidgetFuncDraw draw;
    WidgetFuncFree free;
    WidgetFuncHandleEvent handleEvent;
};

void    setFocus(Widget *widget);
Widget *getFocus(void);

void    setMouseFocus(Widget *widget);
Widget *getMouseFocus(void);

void initWidget(Widget *widget, WidgetStyle *style, WidgetFuncDraw draw, WidgetFuncFree free, WidgetFuncHandleEvent handleEvent);
void freeWidget(Widget *widget);
void drawWidget(Widget *widget, Vector2 offset, Vector2 area);
void handleWidgetEvent(Widget *widget, Event event);

void setMarginX(Widget *widget, float x);
void setMarginY(Widget *widget, float y);

void setDesiredWidth(Widget *widget, float desired_w);
void setDesiredHeight(Widget *widget, float desired_h);
void removeDesiredWidth(Widget *widget);
void removeDesiredHeight(Widget *widget);

Vector2 getDesiredArea(Widget *widget);
float   getDesiredWidth(Widget *widget);
float   getDesiredHeight(Widget *widget);


void    setScrollX(Widget *widget, float x);
void    setScrollY(Widget *widget, float y);
Vector2 getScroll(Widget *widget);
Vector2 getLastDrawArea(Widget *widget);
Vector2 getLastLogicDrawArea(Widget *widget);

#endif