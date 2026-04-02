#ifdef _WIN32

#include "games/snake.h"

#include "arena.h"
#include "defs.h"
#include "event.h"
#include "random.h"
#include "renderer.h"
#include "vec.h"

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

typedef struct
{
    HWND window;
    HDC deviceContext;
    BITMAPINFO bi;
} Win32RendererData;

void PlatformSoftwareRendererPresent(void *platform, u32 *pixels, int width, int height)
{
    Win32RendererData *win32 = platform;
    StretchDIBits(win32->deviceContext, 0, 0, width, height, 0, 0, width, height, pixels, &win32->bi, DIB_RGB_COLORS, SRCCOPY);
}

void PlatformSoftwareRendererInit(void *platform, int width, int height)
{
    Win32RendererData *win32 = platform;

    win32->bi.bmiHeader.biSize = sizeof(win32->bi.bmiHeader);
    win32->bi.bmiHeader.biWidth = width;
    win32->bi.bmiHeader.biHeight = -height;
    win32->bi.bmiHeader.biPlanes = 1;
    win32->bi.bmiHeader.biCompression = BI_RGB;
    win32->bi.bmiHeader.biBitCount = 32;
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch(message)
    {
        case WM_CLOSE:
        {
            DestroyWindow(window);
        } break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;

        default:
        {
            result = DefWindowProcW(window, message, wParam, lParam);
        } break;
    }

    return result;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
    {
        LARGE_INTEGER seedCounter;
        QueryPerformanceCounter(&seedCounter);
        RandSeed(seedCounter.LowPart);
    }

    int clientWidth = 1280;
    int clientHeight = 720;

    WNDCLASSEXW windowClass = { 0 };
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.hInstance = instance;
    windowClass.lpfnWndProc = Win32MainWindowCallback;
    windowClass.lpszClassName = L"RetroGamesWindowClass";

    RegisterClassExW(&windowClass);

    DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

    RECT windowRect = { 0 };
    windowRect.right = clientWidth;
    windowRect.bottom = clientHeight;

    AdjustWindowRectEx(&windowRect, windowStyle, 0, 0);

    Win32RendererData win32 = { 0 };
    win32.window = CreateWindowExW(0, windowClass.lpszClassName, L"Retro Games", windowStyle, CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, 0, 0, instance, 0);
    
    win32.deviceContext = GetDC(win32.window);

    Arena rendererArena = { 0 };
    rendererArena.size = Megabytes(4);
    rendererArena.memory = VirtualAlloc(0, rendererArena.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    Renderer *renderer = RendererInit(&rendererArena, &win32, clientWidth, clientHeight);

    Arena gameArena = { 0 };
    gameArena.size = Megabytes(4);
    gameArena.memory = VirtualAlloc(0, gameArena.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    GameState *gameState = SnakeInit(&gameArena);

    u64 perfCountFreq;

    {
        LARGE_INTEGER perfCountFreqResult;
        QueryPerformanceFrequency(&perfCountFreqResult);
        perfCountFreq = perfCountFreqResult.QuadPart;
    }

    LARGE_INTEGER startCounter;
    QueryPerformanceCounter(&startCounter);

    f32 deltaTime = 0.0f;
    f32 highestDt = 0.0f;
    u64 frames = 0;

    bool running = true;

    while(running)
    {
        MSG message;

        while(PeekMessageW(&message, 0, 0, 0, PM_REMOVE))
        {
            if(message.message == WM_QUIT)
            {
                running = false;
                break;
            }

            Event event = { 0 };

            switch(message.message)
            {
                case WM_KEYUP:
                case WM_KEYDOWN:
                case WM_SYSKEYUP:
                case WM_SYSKEYDOWN:
                {
                    DWORD vkCode = (DWORD)message.wParam;

                    bool wasDown = ((message.lParam & KF_REPEAT) != 0);
                    bool isDown = ((message.lParam & KF_UP) == 0);
                    
                    //We only care about pressing and releasing the key,
                    //this check ignores key repeat messages.
                    if(wasDown != isDown)
                    {
                        event.type = EVENT_KEYBOARD;
                        event.keyboard.isDown = isDown;

                        if(vkCode == VK_UP) event.keyboard.key = KEY_UP;
                        else if(vkCode == VK_DOWN) event.keyboard.key = KEY_DOWN;
                        else if(vkCode == VK_LEFT) event.keyboard.key = KEY_LEFT;
                        else if(vkCode == VK_RIGHT) event.keyboard.key = KEY_RIGHT;
                    }
                } break;

                default:
                {
                    TranslateMessage(&message);
                    DispatchMessageW(&message);
                } break;
            }

            if(event.type != EVENT_NONE) SnakeHandleEvent(gameState, event);
        }

        RendererStartFrame(renderer);

        RendererClear(renderer, COLOR_MAGENTA);

        SnakeUpdate(gameState, renderer, deltaTime);

        RendererEndFrame(renderer);

        LARGE_INTEGER endCounter;
        QueryPerformanceCounter(&endCounter);

        u64 counterElapsed = endCounter.QuadPart - startCounter.QuadPart;
        deltaTime = (f32)counterElapsed / perfCountFreq;
        if(frames < 2000)
        {
            ++frames;
        }
        else if(deltaTime > highestDt)
        {
            highestDt = deltaTime;
            printf("%.03f\n", highestDt);
            fflush(stdout);
        }
        startCounter = endCounter;
    }

    return 0;
}

#endif