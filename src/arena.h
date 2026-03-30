#pragma once

#include "defs.h"

typedef struct
{
    size_t size;
    size_t used;
    u8 *memory;
} Arena;

void *ArenaAlloc(Arena *arena, size_t size, b32 zero);

#define ArenaPushStruct(arena, type) ArenaAlloc(arena, sizeof(type), false)
#define ArenaPushArray(arena, type, count) ArenaAlloc(arena, (count) * sizeof(type), false)
#define ArenaPushStructZero(arena, type) ArenaAlloc(arena, sizeof(type), true)
#define ArenaPushArrayZero(arena, type, count) ArenaAlloc(arena, (count) * sizeof(type), true)