#ifdef _WIN32

#include "defs.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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

    HWND window = CreateWindowExW(0, windowClass.lpszClassName, L"Retro Games", windowStyle, CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, 0, 0, instance, 0);
    
    HDC deviceContext = GetDC(window);
    
    u64 perfCountFreq;

    {
        LARGE_INTEGER perfCountFreqResult;
        QueryPerformanceFrequency(&perfCountFreqResult);
        perfCountFreq = perfCountFreqResult.QuadPart;
    }

    LARGE_INTEGER startCounter;
    QueryPerformanceCounter(&startCounter);

    f32 deltaTime = 0.0f;

    b32 running = true;

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

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        LARGE_INTEGER endCounter;
        QueryPerformanceCounter(&endCounter);

        u64 counterElapsed = endCounter.QuadPart - startCounter.QuadPart;
        deltaTime = (f32)counterElapsed / perfCountFreq;
    }

    return 0;
}

#endif