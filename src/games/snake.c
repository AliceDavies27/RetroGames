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

typedef enum
{
    SEGMENT_HORZ,
    SEGMENT_VERT,
    SEGMENT_LEFT_UP,
    SEGMENT_RIGHT_UP,
    SEGMENT_LEFT_DOWN,
    SEGMENT_RIGHT_DOWN,
    NUM_SEGMENT_TYPES
} SegmentType;

typedef struct
{
    Vec2i pos;
    SegmentType type;
} SnakeSegment;

struct GameState
{
    Arena *arena;

    Vec2 drawRegion;
    Vec2 gridCellSize;

    int gridCount;
    int maxSegments;
    int headIndex, tailIndex;

    SnakeSegment *snakeSegments;
    bool *gridOccupancy;

    Direction lastMove;
    Direction nextMove;

    Vec2i fruitPos;

    f32 accum;
    f32 updateFrequency;

    TexHandle snakeTextures[NUM_SEGMENT_TYPES];

    bool gameOver;
};

inline Vec2 WorldPosToPixelPos(Vec2i pos, Vec2 gridCellSize)
{
    return (Vec2){ (0.5f + pos.x) * gridCellSize.x, (0.5f + pos.y) * gridCellSize.y };
}

static void DrawGridObj(Renderer *renderer, Vec2i pos, Vec2 gridCellSize, Color color)
{
    Vec2 drawPos = {
        (0.5f + pos.x) * gridCellSize.x,
        (0.5f + pos.y) * gridCellSize.y
    };

    Vec2 drawSize = V2SubScalar(gridCellSize, 2);

    RendererFillRect(renderer, drawPos, drawSize, color);
}

static Vec2i GenerateFruit(bool *gridOccupancy, int gridCount)
{
    bool occupied = false;

    //TODO: Generate the position in a better way.
    //This will get slow when the snake is very long.
    for(;;)
    {
        Vec2i pos = {
            RandInt(0, gridCount),
            RandInt(0, gridCount)
        };

        if(!gridOccupancy[pos.y * gridCount + pos.x])
        {
            return pos;
        }
    }
}

GameState *SnakeInit(Arena *arena, Renderer *renderer)
{
    GameState *gameState = ArenaPushStructZero(arena, GameState);

    gameState->arena = arena;

    gameState->drawRegion = (Vec2){ 720, 720 };

    gameState->gridCount = 20;
    gameState->gridCellSize = V2DivScalar(gameState->drawRegion, (f32)gameState->gridCount);

    gameState->maxSegments = gameState->gridCount * gameState->gridCount;
    gameState->snakeSegments = ArenaPushArrayZero(arena, Vec2i, gameState->maxSegments);
    gameState->gridOccupancy = ArenaPushArrayZero(arena, bool, gameState->maxSegments);

    Vec2i pos = { gameState->gridCount / 2, gameState->gridCount / 2 };
    gameState->snakeSegments[0].pos = pos;
    gameState->snakeSegments[0].type = SEGMENT_HORZ;
    gameState->gridOccupancy[pos.y * gameState->gridCount + pos.x] = true;
    pos.x += 1;
    gameState->snakeSegments[1].pos = pos;
    gameState->snakeSegments[1].type = SEGMENT_HORZ;
    gameState->gridOccupancy[pos.y * gameState->gridCount + pos.x] = true;
    pos.x += 1;
    gameState->snakeSegments[2].pos = pos;
    gameState->snakeSegments[2].type = SEGMENT_HORZ;
    gameState->gridOccupancy[pos.y * gameState->gridCount + pos.x] = true;

    gameState->tailIndex = 0;
    gameState->headIndex = 2;

    gameState->lastMove = DIR_RIGHT;
    gameState->nextMove = DIR_RIGHT;

    gameState->fruitPos = GenerateFruit(gameState->gridOccupancy, gameState->gridCount);

    gameState->updateFrequency = 0.2f;

    gameState->snakeTextures[SEGMENT_HORZ] = RendererCreateTexture(renderer, "res/snake_tex_horz.png");
    gameState->snakeTextures[SEGMENT_VERT] = RendererCreateTexture(renderer, "res/snake_tex_vert.png");
    gameState->snakeTextures[SEGMENT_LEFT_UP] = RendererCreateTexture(renderer, "res/snake_tex_left_up.png");
    gameState->snakeTextures[SEGMENT_LEFT_DOWN] = RendererCreateTexture(renderer, "res/snake_tex_left_down.png");
    gameState->snakeTextures[SEGMENT_RIGHT_UP] = RendererCreateTexture(renderer, "res/snake_tex_right_up.png");
    gameState->snakeTextures[SEGMENT_RIGHT_DOWN] = RendererCreateTexture(renderer, "res/snake_tex_right_down.png");

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

        if(!gameState->gameOver)
        {
            SnakeSegment *oldHead = &gameState->snakeSegments[gameState->headIndex % gameState->maxSegments];
            SnakeSegment newHead = *oldHead;

            if(gameState->nextMove == DIR_UP)
            {
                newHead.pos.y--;
                newHead.type = SEGMENT_VERT;

                if(gameState->lastMove == DIR_RIGHT) oldHead->type = SEGMENT_LEFT_UP;
                else if(gameState->lastMove == DIR_LEFT) oldHead->type = SEGMENT_RIGHT_UP;
            }
            else if(gameState->nextMove == DIR_DOWN)
            {
                newHead.pos.y++;
                newHead.type = SEGMENT_VERT;

                if(gameState->lastMove == DIR_RIGHT) oldHead->type = SEGMENT_LEFT_DOWN;
                else if(gameState->lastMove == DIR_LEFT) oldHead->type = SEGMENT_RIGHT_DOWN;
            }
            else if(gameState->nextMove == DIR_LEFT)
            {
                newHead.pos.x--;
                newHead.type = SEGMENT_HORZ;
                if(gameState->lastMove == DIR_UP) oldHead->type = SEGMENT_LEFT_DOWN;
                else if(gameState->lastMove == DIR_DOWN) oldHead->type = SEGMENT_LEFT_UP;
            }
            else if(gameState->nextMove == DIR_RIGHT)
            {
                newHead.pos.x++;
                newHead.type = SEGMENT_HORZ;
                if(gameState->lastMove == DIR_UP) oldHead->type = SEGMENT_RIGHT_DOWN;
                else if(gameState->lastMove == DIR_DOWN) oldHead->type = SEGMENT_RIGHT_UP;
            }

            bool collidedWithEdge = (newHead.pos.x < 0 || newHead.pos.y < 0 ||
                newHead.pos.x >= gameState->gridCount || newHead.pos.y >= gameState->gridCount);
            bool collidedWithSelf = (gameState->gridOccupancy[newHead.pos.y * gameState->gridCount + newHead.pos.x] == true);

            if(collidedWithEdge || collidedWithSelf)
            {
                gameState->gameOver = true;
            }
            else
            {
                if(V2iEqual(newHead.pos, gameState->fruitPos))
                {
                    gameState->fruitPos = GenerateFruit(gameState->gridOccupancy, gameState->gridCount);
                }
                else
                {
                    Vec2i oldTailPos = gameState->snakeSegments[gameState->tailIndex % gameState->maxSegments].pos;
                    gameState->gridOccupancy[oldTailPos.y * gameState->gridCount + oldTailPos.x] = false;
                    gameState->tailIndex++;
                }

                gameState->headIndex++;
                gameState->snakeSegments[gameState->headIndex % gameState->maxSegments] = newHead;
                gameState->gridOccupancy[newHead.pos.y * gameState->gridCount + newHead.pos.x] = true;

                gameState->lastMove = gameState->nextMove;
            }
        }
    }

    RendererSetSize(renderer, gameState->drawRegion);
    RendererFillRect(renderer, V2DivScalar(gameState->drawRegion, 2), gameState->drawRegion, COLOR_BLACK);
    DrawGridObj(renderer, gameState->fruitPos, gameState->gridCellSize, COLOR_RED);

    for(int i = gameState->tailIndex; i <= gameState->headIndex; ++i)
    {
        int segmentIndex = i % gameState->maxSegments;

        SegmentType type = gameState->snakeSegments[segmentIndex].type;
        Vec2 pos = WorldPosToPixelPos(gameState->snakeSegments[segmentIndex].pos, gameState->gridCellSize);

        RendererDrawTexture(renderer, gameState->snakeTextures[type], pos);
    }
}