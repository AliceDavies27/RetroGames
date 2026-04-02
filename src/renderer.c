#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
    Arena arena;
    Arena backendArena;
    Arena assetArena;
    Arena tempArena;

    void *backend;

    Vec2 clientSize;

    struct
    {
        Vec2 virtualSize;
        f32 scale;
        Vec2 offset;
    } transform;

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

    renderer->arena = *arena;
    renderer->arena.size = arena->size / 4;

    renderer->backendArena = renderer->arena;
    renderer->backendArena.memory = renderer->arena.memory + renderer->arena.size;

    renderer->assetArena = renderer->backendArena;
    renderer->assetArena.memory = renderer->backendArena.memory + renderer->backendArena.size;

    renderer->tempArena = renderer->assetArena;
    renderer->tempArena.memory = renderer->assetArena.memory + renderer->assetArena.size;

    renderer->backend = SoftwareRendererInit(&renderer->backendArena, &renderer->api, platform, width, height);
    renderer->clientSize = (Vec2){ (f32)width, (f32)height };
    RendererSetSize(renderer, renderer->clientSize);
    renderer->cmdBuf.size = Kilobytes(64);
    renderer->cmdBuf.memory = ArenaPushArrayZero(arena, u8, renderer->cmdBuf.size);

    return renderer;
}

TexHandle RendererCreateTexture(Renderer *renderer, const char *fileName)
{
    int width, height, channels;
    u32 *tempPixels = (u32 *)stbi_load(fileName, &width, &height, &channels, 0);

    u32 *pixels = ArenaPushArray(&renderer->assetArena, u32, width * height);
    memcpy(pixels, tempPixels, width * height * sizeof(u32));
    
    stbi_image_free(tempPixels);

    return renderer->api.CreateTexture(renderer->backend, width, height, pixels);
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
void RendererSetSize(Renderer *renderer, Vec2 size)
{
    Vec2 scale = V2DivV2(renderer->clientSize, size);

    renderer->transform.virtualSize = size;
    renderer->transform.scale = Min(scale.x, scale.y);
    Vec2 scaledSize = V2MulScalar(size, renderer->transform.scale);
    renderer->transform.offset = V2DivScalar(V2SubV2(renderer->clientSize, scaledSize), 2);
}

void RendererFillRect(Renderer *renderer, Vec2 pos, Vec2 size, Color color)
{
    FillRectCommand *fillRect = CommandBufferPush(&renderer->cmdBuf, COMMAND_FILL_RECT, sizeof(FillRectCommand));

    fillRect->pos = V2AddV2(renderer->transform.offset, V2MulScalar(pos, renderer->transform.scale));
    fillRect->size = V2MulScalar(size, renderer->transform.scale);
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