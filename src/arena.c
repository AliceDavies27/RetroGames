#include "arena.h"

#include <string.h> //For memset.

void *ArenaAlloc(Arena *arena, size_t size, b32 zero)
{
    void *result = 0;

    //Round the size up to a multiple of 8 bytes for alignment.
    size = (size + 7) & ~7;

    if(arena->used + size <= arena->size)
    {
        result = arena->memory + arena->used;

        arena->used += size;

        if(zero)
        {
            memset(result, 0, size);
        }
    }

    return result;
}