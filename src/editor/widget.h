#ifndef SNB_WIDGET_H
#define SNB_WIDGET_H

#include <raylib.h>
#include "event.h"

typedef struct Widget Widget;

struct Widget {
    Widget **parent;
    Vector2 last_area;
    void (*draw)(Widget *widget, Vector2 offset, Vector2 area);
    void (*free)(Widget *widget);
    void (*handleEvent)(Widget *widget, Event event);
};

void setFocus(Widget *widget);
Widget *getFocus(void);
void freeWidget(Widget *widget);
void drawWidget(Widget *widget, Vector2 offset, Vector2 area);
void handleWidgetEvent(Widget *widget, Event event);

#endif