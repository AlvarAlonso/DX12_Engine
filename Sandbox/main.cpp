#include <iostream>

#ifndef UNICODE
#define UNICODE
#endif

#include "utils.h"

#define global_variable static;
#define internal static;
#define local_persist static;

using Microsoft::WRL::ComPtr;
//using namespace std;
//using namespace DirectX;

#define FAILED(hr)      (((HRESULT)(hr)) < 0)

struct DxException
{
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
        : m_errorCode(hr), m_functionName(functionName), m_filename(filename), m_lineNumber(lineNumber) {}
    HRESULT m_errorCode = S_OK;
    std::wstring m_functionName;
    std::wstring m_filename;
    int32_t m_lineNumber = -1;
};

inline std::wstring AnsiToWString(const std::string& str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

#ifndef DX_CHECK
#define DX_CHECK(x)                                                   \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif


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

struct D3DSwapChain
{
    Microsoft::WRL::ComPtr<IDXGISwapChain> handle;
    static const uint32_t bufferCount = 2;
    uint32_t currentBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> imageBuffer[bufferCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
};

struct D3DComponents
{
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    Microsoft::WRL::ComPtr<ID3D12Device> logicalDevice;
    D3DSwapChain swapChain;
} dx12Components;

struct D3DCommands
{
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> directCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
} dx12Commands;

struct WindowImageInfo
{
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    uint32_t clientWidth = 800;
    uint32_t clientHeight = 600;
    D3D12_VIEWPORT screenViewport;
    D3D12_RECT scissorRect;
} windowImageInfo;

struct D3DSyncPrimitives
{
    D3DFence fence;
} dx12SynchPrimitives;

struct D3DFeaturesSupport
{
    bool msaa4xState = false;
    uint32_t msaa4xQuality;
} dx12FeaturesSupport;

ID3D12Resource* CurrentBackBuffer()
{
    return dx12Components.swapChain.imageBuffer[dx12Components.swapChain.currentBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        dx12Components.swapChain.rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        dx12Components.swapChain.currentBackBuffer,
        dx12DescriptorSizes.rtv);
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()
{
    return dx12Components.swapChain.dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

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
    D3D12_COMMAND_QUEUE_DESC queueDescription = {};
    queueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    dx12Components.logicalDevice->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&dx12Commands.commandQueue));

    dx12Components.logicalDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(dx12Commands.directCmdListAlloc.GetAddressOf()));

    dx12Components.logicalDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        dx12Commands.directCmdListAlloc.Get(),
        nullptr,
        IID_PPV_ARGS(dx12Commands.commandList.GetAddressOf()));

    dx12Commands.commandList->Close();

    // create swapchain
    dx12Components.swapChain.handle.Reset();

    DXGI_SWAP_CHAIN_DESC swapChainDescription = {};
    swapChainDescription.BufferDesc.Width = windowImageInfo.clientWidth;
    swapChainDescription.BufferDesc.Height = windowImageInfo.clientHeight;
    swapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDescription.BufferDesc.Format = windowImageInfo.backBufferFormat;
    swapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDescription.SampleDesc.Count = dx12FeaturesSupport.msaa4xState ? 4 : 1;
    swapChainDescription.SampleDesc.Quality = dx12FeaturesSupport.msaa4xState ? (dx12FeaturesSupport.msaa4xQuality - 1) : 0;
    swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescription.BufferCount = dx12Components.swapChain.bufferCount;
    swapChainDescription.OutputWindow = windowHandle;
    swapChainDescription.Windowed = true;
    swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    dx12Components.dxgiFactory->CreateSwapChain(
        dx12Commands.commandQueue.Get(),
        &swapChainDescription,
        dx12Components.swapChain.handle.GetAddressOf());

    // create rtv and dsv descriptor heaps (image views)
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDescriptor = {};
    rtvHeapDescriptor.NumDescriptors = dx12Components.swapChain.bufferCount;
    rtvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDescriptor.NodeMask = 0;

    dx12Components.logicalDevice->CreateDescriptorHeap(
        &rtvHeapDescriptor, 
        IID_PPV_ARGS(dx12Components.swapChain.rtvHeap.GetAddressOf()));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDescriptor = {};
    dsvHeapDescriptor.NumDescriptors = 1;
    dsvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDescriptor.NodeMask = 0;

    dx12Components.logicalDevice->CreateDescriptorHeap(
        &dsvHeapDescriptor,
        IID_PPV_ARGS(dx12Components.swapChain.dsvHeap.GetAddressOf()));

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
            // Draw stuff
            dx12Commands.directCmdListAlloc->Reset();

            dx12Commands.commandList->Reset(dx12Commands.directCmdListAlloc.Get(), nullptr);

            dx12Commands.commandList->ResourceBarrier(
                1, 
                &CD3DX12_RESOURCE_BARRIER::Transition(dx12Components.swapChain.imageBuffer[dx12Components.swapChain.currentBackBuffer].Get(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

            // Set viewport and scissor
            dx12Commands.commandList->RSSetViewports(1, &windowImageInfo.screenViewport);
            dx12Commands.commandList->RSSetScissorRects(1, &windowImageInfo.scissorRect);

            // Clear render targets
            dx12Commands.commandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
            dx12Commands.commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

            dx12Commands.commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
                
            dx12Commands.commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

            dx12Commands.commandList->Close();

            ID3D12CommandList* commandLists = { dx12Commands.commandList.Get() };

            dx12Components.swapChain.handle->Present(0, 0);
            dx12Components.swapChain.currentBackBuffer = (dx12Components.swapChain.currentBackBuffer + 1) % dx12Components.swapChain.bufferCount;

            // Flush command queue
            dx12SynchPrimitives.fence.currentFence++;

            dx12Commands.commandQueue->Signal(dx12SynchPrimitives.fence.handle.Get(), dx12SynchPrimitives.fence.currentFence);

            if(dx12SynchPrimitives.fence.handle->GetCompletedValue() < dx12SynchPrimitives.fence.currentFence)
            {
                HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

                dx12SynchPrimitives.fence.handle->SetEventOnCompletion(dx12SynchPrimitives.fence.currentFence, eventHandle);

                WaitForSingleObject(eventHandle, INFINITE);
                CloseHandle(eventHandle);
            }
        }
    }

    return 0;
}