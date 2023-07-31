#ifndef SNB_WIDGET_H
#define SNB_WIDGET_H

#include <raylib.h>
#include "event.h"

typedef struct Widget Widget;

typedef Vector2 (*WidgetFuncDraw)(Widget *widget, Vector2 offset, Vector2 area);
typedef void    (*WidgetFuncFree)(Widget *widget);
typedef void    (*WidgetFuncHandleEvent)(Widget *widget, Event event);

struct Widget {
    Widget **parent;
    Vector2 last_offset;
    Vector2 last_area;
    Vector2 last_logic_area;
    Vector2 scroll;
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

void initWidget(Widget *widget, WidgetFuncDraw draw, WidgetFuncFree free, WidgetFuncHandleEvent handleEvent);
void freeWidget(Widget *widget);
void drawWidget(Widget *widget, Vector2 offset, Vector2 area);
void handleWidgetEvent(Widget *widget, Event event);

#endif