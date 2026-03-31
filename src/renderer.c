#include "renderer.h"

#include "renderer_api.h"
#include "software_renderer.h"
#include "vec.h"

typedef struct
{
    size_t size;
    size_t used;
    u8 *memory;
} CommandBuffer;

struct Renderer
{
    Arena *arena;
    void *backend;
    CommandBuffer cmdBuf;
    RendererAPI api;
};

typedef enum
{
    COMMAND_CLEAR,
    COMMAND_FILL_RECT,
} CommandType;

typedef struct
{
    CommandType type;
    u32 size;
} CommandHeader;

typedef struct
{
    Color color;
} ClearCommand;

typedef struct
{
    Vec2 pos;
    Vec2 size;
    Color color;
} FillRectCommand;

Renderer *RendererInit(Arena *arena, void *platform, int width, int height)
{
    Renderer *renderer = ArenaPushStructZero(arena, Renderer);

    renderer->arena = arena;
    renderer->backend = SoftwareRendererInit(arena, &renderer->api, platform, width, height);
    renderer->cmdBuf.size = Kilobytes(64);
    renderer->cmdBuf.memory = ArenaPushArrayZero(arena, u8, renderer->cmdBuf.size);

    return renderer;
}

void RendererStartFrame(Renderer *renderer)
{
    renderer->cmdBuf.used = 0;
}

void *CommandBufferPush(CommandBuffer *cmdBuf, CommandType type, u32 size)
{
    u32 commandSize = size + sizeof(CommandHeader);

    Assert(cmdBuf->used + commandSize <= cmdBuf->size);

    CommandHeader *header = (CommandHeader *)(cmdBuf->memory + cmdBuf->used);
    header->type = type;
    header->size = commandSize;

    cmdBuf->used += commandSize;

    return (header + 1);
}

void RendererFillRect(Renderer *renderer, Vec2 pos, Vec2 size, Color color)
{
    FillRectCommand *fillRect = CommandBufferPush(&renderer->cmdBuf, COMMAND_FILL_RECT, sizeof(FillRectCommand));

    fillRect->pos = pos;
    fillRect->size = size;
    fillRect->color = color;
}

void RendererClear(Renderer *renderer, Color color)
{
    ClearCommand *clear = CommandBufferPush(&renderer->cmdBuf, COMMAND_CLEAR, sizeof(ClearCommand));

    clear->color = color;
}

void RendererEndFrame(Renderer *renderer)
{
    u8 *ptr = renderer->cmdBuf.memory;
    u8 *cmdBufEnd = renderer->cmdBuf.memory + renderer->cmdBuf.used;

    while(ptr < cmdBufEnd)
    {
        CommandHeader *header = (CommandHeader *)ptr;
        void *data = (header + 1);

        switch(header->type)
        {
            case COMMAND_CLEAR:
            {
                ClearCommand *clear = data;
                renderer->api.Clear(renderer->backend, clear->color);
            } break;

            case COMMAND_FILL_RECT:
            {
                FillRectCommand *fillRect = data;
                renderer->api.FillRect(renderer->backend, fillRect->pos, fillRect->size, fillRect->color);
            } break;

            default:
            {
                Assert(0);
            } break;
        }

        ptr += header->size;
    }

    renderer->api.Present(renderer->backend);
}