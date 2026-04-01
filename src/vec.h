#pragma once

#include "defs.h"

typedef struct
{
    f32 x, y;
} Vec2;

typedef struct
{
    i32 x, y;
} Vec2i;

inline Vec2 V2AddV2(Vec2 a, Vec2 b) { return (Vec2){ a.x + b.x, a.y + b.y }; }
inline Vec2 V2SubV2(Vec2 a, Vec2 b) { return (Vec2){ a.x - b.x, a.y - b.y }; }
inline Vec2 V2SubScalar(Vec2 v, f32 s) { return (Vec2){ v.x - s, v.y - s }; }
inline Vec2 V2MulScalar(Vec2 v, f32 s) { return (Vec2){ v.x * s, v.y * s }; }
inline Vec2 V2DivV2(Vec2 a, Vec2 b) { return (Vec2){ a.x / b.x, a.y / b.y }; }
inline Vec2 V2DivScalar(Vec2 v, f32 s) { return (Vec2){ v.x / s, v.y / s }; }