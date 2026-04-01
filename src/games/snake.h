#pragma once

#include "arena.h"
#include "event.h"
#include "renderer.h"

typedef struct GameState GameState;

GameState *SnakeInit(Arena *arena);
void SnakeHandleEvent(GameState *gameState, Event event);
void SnakeUpdate(GameState *gameState, Renderer *renderer, f32 deltaTime);