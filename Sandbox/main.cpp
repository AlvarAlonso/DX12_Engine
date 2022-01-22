#include <iostream>

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include "helloWorld.h"

bool m_running = false;

LRESULT CALLBACK
MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (message)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;

        case WM_DESTROY:
        {
            m_running = false;
            OutputDebugStringA("WM_DESTROY\n");
        } break;

        case WM_CLOSE:
        {
            m_running = false;
            OutputDebugStringA("WM_CLOSE\n");
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
            
        default:
        {
            result = DefWindowProc(window, message, wParam, lParam);
        } break;
    }

    return result;
}

int CALLBACK
WinMain(HINSTANCE hInstance, 
        HINSTANCE prevInstance,
		LPSTR lpCmdLine, 
        int showCmd)
{

    WNDCLASS windowClass;
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = MainWindowCallback;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = 0;
    windowClass.hCursor = 0;
    windowClass.hbrBackground = 0;
    windowClass.lpszMenuName = 0;
    windowClass.lpszClassName = L"MainWindow";

    if (!RegisterClass(&windowClass))
    {
        // TODO: handle error
        return false;
    }

    HWND windowHandle = 
    CreateWindowEx(
        0,
        windowClass.lpszClassName,
        L"AlvarEngine",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        hInstance,
        0
    );

    if(windowHandle)
    {
        m_running = true;

        while (m_running)
        {
            MSG message;
            BOOL messageResult = GetMessage(&message, 0, 0, 0);
            if(messageResult > 0)
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        // TODO: handle error
        return false;
    }


    return 0;
}