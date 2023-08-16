#ifndef SPLIT_VIEW_H
#define SPLIT_VIEW_H

#include <stdbool.h>
#include "widget.h"

typedef enum {
    SPLIT_UP,
    SPLIT_DOWN,
    SPLIT_LEFT,
    SPLIT_RIGHT,
} SplitDirection;

bool splitView(WidgetStyle *base_style, SplitDirection dir, Widget *old_widget, Widget *new_widget);

#endif