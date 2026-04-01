#include "random.h"

#include "defs.h"

#include <stdlib.h>

void RandSeed(unsigned int seed)
{
    srand(seed);
}

int RandInt(int min, int max)
{
    return (rand() % (max - min)) + min;
}