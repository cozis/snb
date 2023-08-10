#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include "../utils/basic.h"
#include "group.h"

static void handleEvent(Widget *widget, Event event);
static Vector2 draw(Widget *widget, Vector2 offset, Vector2 area);
static void free_(Widget *widget);

bool insertChildIntoGroup(GroupView *group, Widget *widget)
{
    if (group->children_count == group->children_capacity) {
        
        int capacity;
        
        if (group->children == NULL)
            capacity = 8;
        else
            capacity = 2 * group->children_capacity;
        
        Widget **children2 = realloc(group->children, capacity * sizeof(Widget*));
        if (children2 == NULL)
            return false;

        group->children = children2;
        group->children_capacity = capacity;
    }

    group->children[group->children_count++] = widget;
    return true;
}

bool removeChildFromGroup(GroupView *group, Widget *widget)
{
    int child_index = -1;
    for (int i = 0; i < group->children_count; i++) {
        if (group->children[i] == widget) {
            child_index = i;
            break;
        }
    }
    if (child_index >= 0) {
        for (int i = child_index; i < group->children_count-1; i++)
            group->children[i] = group->children[i+1];
        group->children_count--;
    }
    return child_index >= 0;
}

GroupView *createGroupView(WidgetStyle *base_style)
{
    GroupView *group = malloc(sizeof(GroupView));
    if (group == NULL)
        return NULL;

    initWidget(&group->base, base_style, draw, free_, handleEvent);
    group->children = NULL;
    group->children_count = 0;
    group->children_capacity = 0;
    return group;
}

static void free_(Widget *widget)
{
    GroupView *group = (GroupView*) widget;
    for (int i = 0; i < group->children_count; i++)
        freeWidget(group->children[i]);
    free(group->children);
    free(group);
}

static void handleEvent(Widget *widget, Event event)
{
    GroupView *group = (GroupView*) widget;
    for (int i = 0; i < group->children_count; i++) {
        
        Widget *child = group->children[i];

        if (event.mouse.x >= child->last_offset.x && event.mouse.x < child->last_offset.x + child->last_area.x &&
            event.mouse.y >= child->last_offset.y && event.mouse.y < child->last_offset.y + child->last_area.y) {

            event.mouse.x -= child->last_offset.x;
            event.mouse.y -= child->last_offset.y;
            handleWidgetEvent(child, event);
        }
    }
}

static Vector2 draw(Widget *widget, Vector2 offset, Vector2 area)
{
    GroupView *group = (GroupView*) widget;

    Vector2 current_offset = offset;
    float max_line_h = 0;
    float max_line_w = 0;
    
    for (int i = 0; i < group->children_count; i++) {
        
        Widget *child = group->children[i];

        Vector2 margin = child->margin;
        Vector2 desired_area = getDesiredArea(child);
        
        if (current_offset.x + margin.x + desired_area.x > area.x) {
            // Break to the next line
            current_offset.x = offset.x;
            current_offset.y += max_line_h;
            max_line_w = MAX(max_line_w, current_offset.x - offset.x);
            max_line_h = 0;
        }

        Vector2 offset;
        offset.x = current_offset.x + margin.y;
        offset.y = current_offset.y + margin.y;
        
        drawWidget(child, offset, desired_area);

        current_offset.x += margin.x + desired_area.x;
        max_line_h = MAX(max_line_h, margin.y + desired_area.y);
    }
    current_offset.y += max_line_h;
    
    Vector2 used_area;
    used_area.x = max_line_w;
    used_area.y = current_offset.y - offset.y;
    return used_area;
}
