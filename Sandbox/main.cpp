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

struct D3DFence
{
    Microsoft::WRL::ComPtr<ID3D12Fence> handle;
    uint64_t currentFence = 0;
};

struct D3DDescriptorSizes
{
    uint32_t rtv;
    uint32_t dsv;
    uint32_t srv_uav_cbv;
} dx12DescriptorSizes;

struct D3DComponents
{
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> logicalDevice;
} dx12Components;

struct WindowImageInfo
{
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    uint32_t clientWidth = 800;
    uint32_t clientHeight = 600;
} windowImageInfo;

struct D3DShyncPrimitives
{
    D3DFence fence;
} dx12SynchPrimitives;

struct D3DFeaturesSupport
{
    uint32_t msaa4xQuality;
} dx12FeaturesSupport;

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

    // synchcronitzation primitives
    dx12Components.logicalDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&dx12SynchPrimitives.fence.handle));

    // descriptor sizes
    dx12DescriptorSizes.rtv = dx12Components.logicalDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dx12DescriptorSizes.dsv = dx12Components.logicalDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    dx12DescriptorSizes.srv_uav_cbv = dx12Components.logicalDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Check 4x MSAA quality support
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
    msaaQualityLevels.Format = windowImageInfo.backBufferFormat;
    msaaQualityLevels.SampleCount = 4;
    msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msaaQualityLevels.NumQualityLevels = 0;

    dx12Components.logicalDevice->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msaaQualityLevels,
        sizeof(msaaQualityLevels));

    dx12FeaturesSupport.msaa4xQuality = msaaQualityLevels.NumQualityLevels;
    assert(dx12FeaturesSupport.msaa4xQuality > 0);

    // create command objects

    // create swapchain

    // create rtv and dsv descriptor heaps

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