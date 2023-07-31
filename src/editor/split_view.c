#include <assert.h>
#include <stddef.h>
#include "split_view.h"

typedef enum {
    SPLIT_X,
    SPLIT_Y,
} SplitAxis;

typedef struct {
    Widget base;
    SplitAxis axis;
    float left_ratio;
    Widget *left_or_up;
    Widget *right_or_down;
} SplitView;

typedef union SplitViewStub SplitViewStub;

union SplitViewStub {
    SplitViewStub *next;
    SplitView split;
};

#define MAX_SPLITS 8

static bool initialized = false;
static SplitViewStub *free_list;
static SplitViewStub pool[MAX_SPLITS];

static SplitView *getStructMemory(void)
{
    if (!initialized) {
        free_list = pool;
        for (int i = 0; i < MAX_SPLITS-1; i++)
            pool[i].next = &pool[i+1];
        pool[MAX_SPLITS-1].next = NULL;
        initialized = true;
    }

    if (free_list == NULL)
        return NULL;

    SplitViewStub *stub = free_list;
    free_list = stub->next;

    return &stub->split;
}

static void freeStructMemory(SplitView *split)
{
    assert(initialized);
    SplitViewStub *stub = (SplitViewStub*) split;
    stub->next = free_list;
    free_list = stub;
}

static void handleEvent(Widget *widget, Event event);
static Vector2 draw(Widget *widget, Vector2 offset, Vector2 area);
static void free_(Widget *widget);

bool splitView(SplitDirection dir, Widget *old_widget, Widget *new_widget)
{
    SplitView *split = getStructMemory();
    if (split == NULL)
        return false;

    Widget *left_or_up;
    Widget *right_or_down;

    SplitAxis axis = SPLIT_Y;
    switch (dir) {

        case SPLIT_LEFT:
            axis = SPLIT_X;
            /* fallthrough */
        case SPLIT_UP:
            left_or_up = new_widget;
            right_or_down = old_widget;
            break;

        case SPLIT_RIGHT:
            axis = SPLIT_X;
            /* fallthrough */
        case SPLIT_DOWN:
            left_or_up = old_widget;
            right_or_down = new_widget;
            break;
    }

    initWidget(&split->base, draw, free_, handleEvent);
    split->axis = axis;
    split->left_ratio = 0.5;
    split->left_or_up = left_or_up;
    split->right_or_down = right_or_down;

    *old_widget->parent = (Widget*) split;
    new_widget->parent = (new_widget == left_or_up) ? &split->left_or_up : &split->right_or_down;
    old_widget->parent = (old_widget == left_or_up) ? &split->left_or_up : &split->right_or_down;
    return true;
}

static void free_(Widget *widget)
{
    SplitView *split = (SplitView*) widget;
    freeWidget(split->left_or_up);
    freeWidget(split->right_or_down);
    freeStructMemory(split);
}

static void handleEvent(Widget *widget, Event event)
{
    SplitView *split = (SplitView*) widget;
    
    switch (split->axis) {
        
        case SPLIT_X:
        if (event.mouse.x < split->base.last_area.x * split->left_ratio)
            handleWidgetEvent(split->left_or_up, event);
        else {
            event.mouse.x -= split->base.last_area.x * split->left_ratio;
            handleWidgetEvent(split->right_or_down, event);
        }
        break;
        
        case SPLIT_Y:
        if (event.mouse.y < split->base.last_area.y * split->left_ratio)
            handleWidgetEvent(split->left_or_up, event);
        else {
            event.mouse.y -= split->base.last_area.y * split->left_ratio;
            handleWidgetEvent(split->right_or_down, event);
        }
        break;
    }
}

static Vector2 draw(Widget *widget, Vector2 offset, Vector2 area)
{
    SplitView *split = (SplitView*) widget;

    Vector2 l_offset, l_area;
    Vector2 r_offset, r_area;

    switch (split->axis) {

        case SPLIT_X:
        l_offset.x = offset.x;
        l_offset.y = offset.y;
        l_area.x = area.x * split->left_ratio;
        l_area.y = area.y;
        r_offset.x = l_offset.x + l_area.x;
        r_offset.y = offset.y;
        r_area.x = area.x - l_area.x;
        r_area.y = area.y;
        break;

        case SPLIT_Y:
        l_offset.x = offset.x;
        l_offset.y = offset.y;
        l_area.x = area.x;
        l_area.y = area.y * split->left_ratio;
        r_offset.x = offset.x;
        r_offset.y = l_offset.y + l_area.y;
        r_area.x = area.x;
        r_area.y = area.y - l_area.y;
        break;
    }

    drawWidget(split->left_or_up,    l_offset, l_area);
    drawWidget(split->right_or_down, r_offset, r_area);

    switch (split->axis) {
        case SPLIT_X: DrawLine(l_offset.x + l_area.x, l_offset.y, l_offset.x + l_area.x, l_offset.y + l_area.y, BLACK); break;
        case SPLIT_Y: DrawLine(l_offset.x, l_offset.y + l_area.y, l_offset.x + l_area.x, l_offset.y + l_area.y, BLACK); break;
    }

    return area;
}
