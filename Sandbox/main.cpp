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

#define global_variable static;
#define internal static;
#define local_persist static;

using Microsoft::WRL::ComPtr;
//using namespace std;
//using namespace DirectX;


global_variable bool running = false;

struct Direct3DComponents
{
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> logicalDevice;
} dx12Components;

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
            running = false;
            OutputDebugStringA("WM_DESTROY\n");
        } break;

        case WM_CLOSE:
        {
            running = false;
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

    if(!windowHandle)
    {
        // TODO: handle error
        return false;
    }

    // init Direct3D 12
    // TODO: Manage possible errors
    CreateDXGIFactory1(IID_PPV_ARGS(&dx12Components.dxgiFactory));

    // Get hardware adapter
    // TODO: Enumerate hardware adapters and pick the most suitable one
    HRESULT result = D3D12CreateDevice(
        nullptr, 
        D3D_FEATURE_LEVEL_11_0, 
        IID_PPV_ARGS(&dx12Components.logicalDevice));

    // If no hardware device, get WARP device
    if (result < 0)
    {
        // TODO: Warn the user about not found hardware device
        ComPtr<IDXGIAdapter> pWarpAdapter;
        dx12Components.dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));

        D3D12CreateDevice(
            pWarpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&dx12Components.logicalDevice));
    }

    // main loop
    running = true;

    while (running)
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

    return 0;
}