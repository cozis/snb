#include <stddef.h>
#include "widget.h"

void drawWidget(Widget *widget, Vector2 offset, Vector2 area)
{
    DrawRectangle(offset.x, offset.y, area.x, area.y, WHITE);
    widget->draw(widget, offset, area);
    widget->last_area = area;
}

void handleWidgetEvent(Widget *widget, Event event)
{
    widget->handleEvent(widget, event);
}

static Widget *focus = NULL;

void freeWidget(Widget *widget)
{
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