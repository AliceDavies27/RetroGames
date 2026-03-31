#pragma once

#include "arena.h"
#include "renderer_types.h"
#include "vec.h"

typedef struct Renderer Renderer;

Renderer *RendererInit(Arena *arena, void *platform, int width, int height);

void RendererStartFrame(Renderer *renderer);

void RendererSetSize(Renderer *renderer, Vec2 size);
void RendererClear(Renderer *renderer, Color color);
void RendererFillRect(Renderer *renderer, Vec2 pos, Vec2 size, Color color);

void RendererEndFrame(Renderer *renderer);