#ifndef SNB_EVENT_H
#define SNB_EVENT_H

#include <raylib.h>

typedef enum {
    EVENT_KEY,
    EVENT_TEXT,
    EVENT_OPEN,
    EVENT_MOUSE_LEFT_UP,
    EVENT_MOUSE_LEFT_DOWN,
} EventType;

typedef struct {
    EventType type;
    Vector2  mouse;
    union {
        int rune;
        int key;
        const char *path;
    };
} Event;

#endif