#pragma once

#include "defs.h"

#ifdef _WIN32
#include <intrin.h>
#endif

inline u64 CountLeadingZeros64(u64 value)
{
    #if 0
    return __lzcnt64(value);
    #else
    if(!value) return 64;
    for(int i = 63; i >= 0; ++i)
    {
        if(value & (1LL << i)) return 63 - i;
    }
    #endif
}