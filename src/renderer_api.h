#pragma once

#include "renderer_types.h"
#include "vec.h"

typedef struct
{
    TexHandle (*CreateTexture)(void *backend, int width, int height, u32 *pixels);
    
    void (*Clear)(void *backend, Color color);
    void (*FillRect)(void *backend, Vec2 pos, Vec2 size, Color color);
    void (*DrawTexture)(void *backend, TexHandle texHandle, Vec2 pos);

    void (*Present)();
} RendererAPI;