#pragma once

#include "arena.h"
#include "renderer_api.h"

void *SoftwareRendererInit(Arena *arena, RendererAPI *api, void *platform, int width, int height);