#include "snake.h"

#include "random.h"
#include "vec.h"

typedef enum
{
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

struct GameState
{
    Arena *arena;

    Vec2 drawRegion;

    int gridCount;
    Vec2 gridCellSize;

    Vec2i *snakeSegments;
    b32 *gridOccupancy;
    int maxSegments;
    int headIndex, tailIndex;
    Direction lastMove;
    Direction nextMove;

    Vec2i fruitPos;

    f32 accum;
    f32 updateFrequency;
};

static void DrawGridObj(Renderer *renderer, Vec2 gridCellSize, Vec2i pos, Color color)
{
    Vec2 drawPos = {
        (0.5f + pos.x) * gridCellSize.x,
        (0.5f + pos.y) * gridCellSize.y
    };

    Vec2 drawSize = V2SubScalar(gridCellSize, 2);

    RendererFillRect(renderer, drawPos, drawSize, color);
}

GameState *SnakeInit(Arena *arena)
{
    GameState *gameState = ArenaPushStructZero(arena, GameState);

    gameState->arena = arena;

    gameState->drawRegion = (Vec2){ 720, 720 };

    gameState->gridCount = 20;
    gameState->gridCellSize = V2DivScalar(gameState->drawRegion, (f32)gameState->gridCount);

    gameState->maxSegments = gameState->gridCount * gameState->gridCount;
    gameState->snakeSegments = ArenaPushArrayZero(arena, Vec2i, gameState->maxSegments);
    gameState->gridOccupancy = ArenaPushArrayZero(arena, b32, gameState->maxSegments);

    Vec2i pos = { gameState->gridCount / 2, gameState->gridCount / 2 };
    gameState->snakeSegments[0] = pos;
    gameState->gridOccupancy[pos.y * gameState->gridCount + pos.x] = true;
    pos.x += 1;
    gameState->snakeSegments[1] = pos;
    gameState->gridOccupancy[pos.y * gameState->gridCount + pos.x] = true;
    pos.x += 1;
    gameState->snakeSegments[2] = pos;
    gameState->gridOccupancy[pos.y * gameState->gridCount + pos.x] = true;

    gameState->tailIndex = 0;
    gameState->headIndex = 2;

    gameState->lastMove = DIR_RIGHT;
    gameState->nextMove = DIR_RIGHT;

    b32 occupied = false;

    for(;;)
    {
        pos = (Vec2i) {
            RandInt(0, gameState->gridCount),
            RandInt(0, gameState->gridCount)
        };

        if(!gameState->gridOccupancy[pos.y * gameState->gridCount + pos.x])
        {
            gameState->fruitPos = pos;
            break;
        }
    }

    gameState->updateFrequency = 0.2f;

    return gameState;
}

void SnakeHandleEvent(GameState *gameState, Event event)
{
    if(event.type == EVENT_KEYBOARD)
    {
        switch(event.keyboard.key)
        {
            case KEY_UP:
            {
                if(gameState->lastMove != DIR_DOWN) gameState->nextMove = DIR_UP;
            } break;

            case KEY_DOWN:
            {
                if(gameState->lastMove != DIR_UP) gameState->nextMove = DIR_DOWN;
            } break;

            case KEY_LEFT:
            {
                if(gameState->lastMove != DIR_RIGHT) gameState->nextMove = DIR_LEFT;
            } break;

            case KEY_RIGHT:
            {
                if(gameState->lastMove != DIR_LEFT) gameState->nextMove = DIR_RIGHT;
            } break;
        }
    }
}

void SnakeUpdate(GameState *gameState, Renderer *renderer, f32 deltaTime)
{
    gameState->accum += deltaTime;

    if(gameState->accum > gameState->updateFrequency)
    {
        gameState->accum -= gameState->updateFrequency;

        Vec2i newHeadPos = gameState->snakeSegments[gameState->headIndex % gameState->maxSegments];

        if(gameState->nextMove == DIR_UP) newHeadPos.y--;
        else if(gameState->nextMove == DIR_DOWN) newHeadPos.y++;
        else if(gameState->nextMove == DIR_LEFT) newHeadPos.x--;
        else if(gameState->nextMove == DIR_RIGHT) newHeadPos.x++;

        gameState->headIndex++;
        gameState->snakeSegments[gameState->headIndex % gameState->maxSegments] = newHeadPos;
        gameState->tailIndex++;

        gameState->lastMove = gameState->nextMove;
    }

    RendererSetSize(renderer, gameState->drawRegion);
    RendererFillRect(renderer, V2DivScalar(gameState->drawRegion, 2), gameState->drawRegion, COLOR_BLACK);
    DrawGridObj(renderer, gameState->gridCellSize, gameState->fruitPos, COLOR_RED);

    for(int i = gameState->tailIndex; i <= gameState->headIndex; ++i)
    {
        int segmentIndex = i % gameState->maxSegments;
        DrawGridObj(renderer, gameState->gridCellSize, gameState->snakeSegments[segmentIndex], COLOR_WHITE);
    }
}