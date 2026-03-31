#pragma once

#include "renderer_types.h"
#include "vec.h"

typedef struct
{
    void (*Clear)(void *backend, Color color);
    void (*FillRect)(void *backend, Vec2 pos, Vec2 size, Color color);

    void (*Present)();
} RendererAPI;