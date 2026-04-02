#include "software_renderer.h"

#include "defs.h"
#include "intrinsics.h"
#include "vec.h"

#define MAX_TEXTURES 128

typedef struct
{
    int width, height;
    u32 *pixels;
} SWTexture;

typedef struct
{
    Arena *arena;
    void *platform;
    u32 *pixels;
    int width, height;

    u32 activeTextures;
    SWTexture textures[MAX_TEXTURES];
} SoftwareRenderer;

void PlatformSoftwareRendererPresent(void *platform, u32 *pixels, int width, int height);
void PlatformSoftwareRendererInit(void *platform, int width, int height);

static u32 ColorToU32(Color color)
{
    u32 r = (u32)(color.r * 255.0f);
    u32 g = (u32)(color.g * 255.0f);
    u32 b = (u32)(color.b * 255.0f);
    u32 a = (u32)(color.a * 255.0f);

    u32 pixColor = (a << 24) | (r << 16) | (g << 8) | b;

    return pixColor;
}

static TexHandle CreateTexture(void *backend, int width, int height, u32 *pixels)
{
    SoftwareRenderer *sw = backend;

    TexHandle texHandle = INVALID_HANDLE;

    if(sw->activeTextures < MAX_TEXTURES)
    {
        texHandle = sw->activeTextures++;

        SWTexture *texture = &sw->textures[texHandle];
        
        texture->width = width;
        texture->height = height;
        texture->pixels = pixels;

    }

    return texHandle;
}

static void Clear(void *backend, Color color)
{
    SoftwareRenderer *sw = backend;

    u32 pixColor = ColorToU32(color);

    for(int i = 0; i < sw->width * sw->height; ++i)
    {
        sw->pixels[i] = pixColor;
    }
}

static void FillRect(void *backend, Vec2 pos, Vec2 size, Color color)
{
    SoftwareRenderer *sw = backend;
    
    i32 minX = Max((i32)(pos.x - size.x * 0.5f), 0);
    i32 minY = Max((i32)(pos.y - size.y * 0.5f), 0);
    i32 maxX = Min((i32)(pos.x + size.x * 0.5f), sw->width);
    i32 maxY = Min((i32)(pos.y + size.y * 0.5f), sw->height);

    u32 pixColor = ColorToU32(color);

    for(int y = minY; y < maxY; ++y)
    {
        for(int x = minX; x < maxX; ++x)
        {
            sw->pixels[y * sw->width + x] = pixColor;
        }
    }
}

static void Present(void *backend)
{
    SoftwareRenderer *sw = backend;
    PlatformSoftwareRendererPresent(sw->platform, sw->pixels, sw->width, sw->height);
}

void *SoftwareRendererInit(Arena *arena, RendererAPI *api, void *platform, int width, int height)
{
    SoftwareRenderer *sw = ArenaPushStructZero(arena, SoftwareRenderer);

    PlatformSoftwareRendererInit(platform, width, height);

    sw->arena = arena;
    sw->platform = platform;
    sw->pixels = ArenaPushArrayZero(arena, u32, width * height);
    sw->width = width;
    sw->height = height;

    api->CreateTexture = CreateTexture;

    api->Clear = Clear;
    api->FillRect = FillRect;

    api->Present = Present;

    return sw;
}