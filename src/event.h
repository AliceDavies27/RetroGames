#pragma once

#include "defs.h"

typedef enum
{
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT
} KeyCode;

typedef struct
{
    KeyCode key;
    b32 isDown;
} KeyboardEvent;

typedef enum
{
    EVENT_NONE = 0,
    EVENT_KEYBOARD
} EventType;

typedef struct
{
    EventType type;

    union
    {
        KeyboardEvent keyboard;
    };
} Event;