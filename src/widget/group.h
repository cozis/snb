#include <stdbool.h>
#include "widget.h"

typedef struct {
    Widget base;
    Widget **children;
    int children_count;
    int children_capacity;
} GroupView;

GroupView *createGroupView(WidgetStyle *base_style);
bool insertChildIntoGroup(GroupView *group, Widget *widget);
bool removeChildFromGroup(GroupView *group, Widget *widget);
