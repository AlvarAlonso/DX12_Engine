#include <iostream>

#ifndef UNICODE
#define UNICODE
#endif

#include "defines.h"
#include "utils.h"
#include "logger.h"

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
//std::cout << "[ERROR]"<< hr__ <<std::endl;
//if(FAILED(hr__)) { std::cout << "[ERROR]"<< hr__ <<std::endl; }    \        

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
};

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
};

struct D3DCommands
{
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> directCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
};

struct WindowImageInfo
{
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    uint32_t clientWidth = 800;
    uint32_t clientHeight = 600;
    D3D12_VIEWPORT screenViewport;
    D3D12_RECT scissorRect;
};

struct D3DSyncPrimitives
{
    D3DFence fence;
};

struct D3DFeaturesSupport
{
    bool msaa4xState = false;
    uint32_t msaa4xQuality;
};

struct D3DApp
{
    bool running = false;
    HWND windowHandle;
    HINSTANCE hInstance;
} d3dApp;

struct D3DDevice
{
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    Microsoft::WRL::ComPtr<ID3D12Device> logicalDevice;

    D3DFeaturesSupport features;
    D3DDescriptorSizes descriptorSizes;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> directCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
};

struct D3DState
{
    D3DDevice device;
    D3DSwapChain swapChain;

    D3DFence fence;

    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    uint32_t clientWidth = 800;
    uint32_t clientHeight = 600;
    D3D12_VIEWPORT screenViewport;
    D3D12_RECT scissorRect;

} D3DState;

ID3D12Resource* CurrentBackBuffer()
{
    return D3DState.swapChain.imageBuffer[D3DState.swapChain.currentBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        D3DState.swapChain.rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        D3DState.swapChain.currentBackBuffer,
        D3DState.device.descriptorSizes.rtv);
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()
{
    return D3DState.swapChain.dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

LRESULT CALLBACK
MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (message)
    {
        case WM_SIZE:
        {
            logOutput(LOG_LEVEL_INFO, "WM_SIZE\n");
            //OutputDebugStringA("WM_SIZE\n");
        } break;

        case WM_DESTROY:
        {
            d3dApp.running = false;
            logOutput(LOG_LEVEL_INFO, "WM_DESTROY\n");
            //OutputDebugStringA("WM_DESTROY\n");
        } break;

        case WM_CLOSE:
        {
            d3dApp.running = false;
            logOutput(LOG_LEVEL_INFO, "WM_CLOSE\n");
            //OutputDebugStringA("WM_CLOSE\n");
        } break;

        case WM_ACTIVATEAPP:
        {
            logOutput(LOG_LEVEL_INFO, "WM_ACTIVATEAPP\n");
            //OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
            
        default:
        {
            result = DefWindowProc(window, message, wParam, lParam);
        } break;
    }

    return result;
}

bool InitWindow()
{
    WNDCLASS windowClass;
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = MainWindowCallback;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = d3dApp.hInstance;
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

    d3dApp.windowHandle = 
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
        d3dApp.hInstance,
        0
    );

    if(!d3dApp.windowHandle)
    {
        // TODO: handle error
        return false;
    }

    return true;
}

void InitDirect3D()
{
   // init Direct3D 12
    ComPtr<ID3D12Debug> debugController;
    D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
    debugController->EnableDebugLayer();

    // TODO: Manage possible errors
    DX_CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&D3DState.device.dxgiFactory)));

    // Get hardware adapter
    // TODO: Enumerate hardware adapters and pick the most suitable one
    HRESULT deviceResult = D3D12CreateDevice(
        nullptr, 
        D3D_FEATURE_LEVEL_11_0, 
        IID_PPV_ARGS(&D3DState.device.logicalDevice));

    // If no hardware device, get WARP device
    if (FAILED(deviceResult))
    {
        // TODO: Warn the user about not found hardware device
        ComPtr<IDXGIAdapter> pWarpAdapter;
        D3DState.device.dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));

        DX_CHECK(D3D12CreateDevice(
            pWarpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&D3DState.device.logicalDevice)));
    }

    // synchcronitzation primitives
    DX_CHECK(D3DState.device.logicalDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&D3DState.fence.handle)));

    // descriptor sizes
    D3DState.device.descriptorSizes.rtv = D3DState.device.logicalDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3DState.device.descriptorSizes.dsv = D3DState.device.logicalDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    D3DState.device.descriptorSizes.srv_uav_cbv = D3DState.device.logicalDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Check 4x MSAA quality support
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
    msaaQualityLevels.Format = D3DState.backBufferFormat;
    msaaQualityLevels.SampleCount = 4;
    msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msaaQualityLevels.NumQualityLevels = 0;

    DX_CHECK(D3DState.device.logicalDevice->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msaaQualityLevels,
        sizeof(msaaQualityLevels)));

    D3DState.device.features.msaa4xQuality = msaaQualityLevels.NumQualityLevels;
    assert(D3DState.device.features.msaa4xQuality > 0);

    // create command objects
    D3D12_COMMAND_QUEUE_DESC queueDescription = {};
    queueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    DX_CHECK(D3DState.device.logicalDevice->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&D3DState.device.commandQueue)));

    DX_CHECK(D3DState.device.logicalDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(D3DState.device.directCmdListAlloc.GetAddressOf())));

    DX_CHECK(D3DState.device.logicalDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        D3DState.device.directCmdListAlloc.Get(),
        nullptr,
        IID_PPV_ARGS(D3DState.device.commandList.GetAddressOf())));

    D3DState.device.commandList->Close();

    // create swapchain
    D3DState.swapChain.handle.Reset();

    DXGI_SWAP_CHAIN_DESC swapChainDescription = {};
    swapChainDescription.BufferDesc.Width = D3DState.clientWidth;
    swapChainDescription.BufferDesc.Height = D3DState.clientHeight;
    swapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDescription.BufferDesc.Format = D3DState.backBufferFormat;
    swapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDescription.SampleDesc.Count = D3DState.device.features.msaa4xState ? 4 : 1;
    swapChainDescription.SampleDesc.Quality = D3DState.device.features.msaa4xState ? (D3DState.device.features.msaa4xQuality - 1) : 0;
    swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescription.BufferCount = D3DState.swapChain.bufferCount;
    swapChainDescription.OutputWindow = d3dApp.windowHandle;
    swapChainDescription.Windowed = true;
    swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    HRESULT result;
    DX_CHECK(D3DState.device.dxgiFactory->CreateSwapChain(
        D3DState.device.commandQueue.Get(),
        &swapChainDescription,
        D3DState.swapChain.handle.GetAddressOf()));

    // create rtv and dsv descriptor heaps (image views)
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDescriptor = {};
    rtvHeapDescriptor.NumDescriptors = D3DState.swapChain.bufferCount;
    rtvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDescriptor.NodeMask = 0;

    DX_CHECK(D3DState.device.logicalDevice->CreateDescriptorHeap(
        &rtvHeapDescriptor, 
        IID_PPV_ARGS(D3DState.swapChain.rtvHeap.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDescriptor = {};
    dsvHeapDescriptor.NumDescriptors = 1;
    dsvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDescriptor.NodeMask = 0;

    DX_CHECK(D3DState.device.logicalDevice->CreateDescriptorHeap(
        &dsvHeapDescriptor,
        IID_PPV_ARGS(D3DState.swapChain.dsvHeap.GetAddressOf())));

    // resize code
    assert(D3DState.device.logicalDevice);
    assert(D3DState.swapChain.handle);
    assert(D3DState.device.directCmdListAlloc);

    // flush command queue
    D3DState.fence.currentFence++;

    D3DState.device.commandQueue->Signal(D3DState.fence.handle.Get(), D3DState.fence.currentFence);

    if(D3DState.fence.handle->GetCompletedValue() < D3DState.fence.currentFence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

        D3DState.fence.handle->SetEventOnCompletion(D3DState.fence.currentFence, eventHandle);

        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    DX_CHECK(D3DState.device.commandList->Reset(D3DState.device.directCmdListAlloc.Get(), nullptr));

    // release swapchain resources
    for (int i = 0; i < D3DState.swapChain.bufferCount; i++)
    {
        D3DState.swapChain.imageBuffer[i].Reset();
    }

    D3DState.swapChain.depthStencilBuffer.Reset();

    // resize the swapchain
    DX_CHECK(D3DState.swapChain.handle->ResizeBuffers(
        D3DState.swapChain.bufferCount,
        D3DState.clientWidth, D3DState.clientHeight,
        D3DState.backBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    D3DState.swapChain.currentBackBuffer = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(D3DState.swapChain.rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (uint32 i = 0; i < D3DState.swapChain.bufferCount; i++)
    {
        DX_CHECK(D3DState.swapChain.handle->GetBuffer(i, IID_PPV_ARGS(&D3DState.swapChain.imageBuffer[i])));
        D3DState.device.logicalDevice->CreateRenderTargetView(D3DState.swapChain.imageBuffer[i].Get(), nullptr, rtvHeapHandle);
        rtvHeapHandle.Offset(1, D3DState.device.descriptorSizes.rtv);
    }

    // create the depth/stencil buffer and view
    D3D12_RESOURCE_DESC depthStencilDescription = {};
    depthStencilDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDescription.Alignment = 0;
    depthStencilDescription.Width = D3DState.clientWidth;
    depthStencilDescription.Height = D3DState.clientHeight;
    depthStencilDescription.DepthOrArraySize = 1;
    depthStencilDescription.MipLevels = 1;
    depthStencilDescription.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthStencilDescription.SampleDesc.Count = D3DState.device.features.msaa4xState ? 4 : 1;
    depthStencilDescription.SampleDesc.Quality = D3DState.device.features.msaa4xState ? (D3DState.device.features.msaa4xState - 1) : 0;
    depthStencilDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = D3DState.depthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    DX_CHECK(D3DState.device.logicalDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDescription,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(D3DState.swapChain.depthStencilBuffer.GetAddressOf())));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvViewDescription = {};
    dsvViewDescription.Flags = D3D12_DSV_FLAG_NONE;
    dsvViewDescription.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvViewDescription.Format = D3DState.depthStencilFormat;
    dsvViewDescription.Texture2D.MipSlice = 0;
    D3DState.device.logicalDevice->CreateDepthStencilView(D3DState.swapChain.depthStencilBuffer.Get(), &dsvViewDescription, DepthStencilView());

    // transition the resource from its initial state to be used as a depth buffer
    D3DState.device.commandList->ResourceBarrier(
        1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            D3DState.swapChain.depthStencilBuffer.Get(),
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

    // execute resize commands
    DX_CHECK(D3DState.device.commandList->Close());
    ID3D12CommandList* cmdsLists[] = { D3DState.device.commandList.Get() };
    D3DState.device.commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // flush command queue
    D3DState.fence.currentFence++;

    DX_CHECK(D3DState.device.commandQueue->Signal(D3DState.fence.handle.Get(), D3DState.fence.currentFence));

    if(D3DState.fence.handle->GetCompletedValue() < D3DState.fence.currentFence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

        DX_CHECK(D3DState.fence.handle->SetEventOnCompletion(D3DState.fence.currentFence, eventHandle));

        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    // update the viewport transform to cover the client area
    D3DState.screenViewport.TopLeftX = 0;
    D3DState.screenViewport.TopLeftY = 0;
    D3DState.screenViewport.Width = static_cast<float32>(D3DState.clientWidth);
    D3DState.screenViewport.Height = static_cast<float32>(D3DState.clientHeight);
    D3DState.screenViewport.MinDepth = 0.0f;
    D3DState.screenViewport.MaxDepth = 1.0f;

    D3DState.scissorRect = { 0, 0, static_cast<long>(D3DState.clientWidth), static_cast<long>(D3DState.clientHeight) };

}

void DrawFrame()
{
    // Draw stuff
    D3DState.device.directCmdListAlloc->Reset();

    D3DState.device.commandList->Reset(D3DState.device.directCmdListAlloc.Get(), nullptr);

    ID3D12Resource* resource = D3DState.swapChain.imageBuffer[D3DState.swapChain.currentBackBuffer].Get();

    const D3D12_RESOURCE_BARRIER* barrier = &CD3DX12_RESOURCE_BARRIER::Transition(
        resource,
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    D3DState.device.commandList->ResourceBarrier(
        1, 
        barrier);

    // Set viewport and scissor
    D3DState.device.commandList->RSSetViewports(1, &D3DState.screenViewport);
    D3DState.device.commandList->RSSetScissorRects(1, &D3DState.scissorRect);

    // Clear render targets
    D3DState.device.commandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
    D3DState.device.commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    D3DState.device.commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
        
    D3DState.device.commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    DX_CHECK(D3DState.device.commandList->Close());

    // execute commands
    ID3D12CommandList* commandLists[] = { D3DState.device.commandList.Get() };
    D3DState.device.commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    // swap buffers
    D3DState.swapChain.handle->Present(0, 0);
    D3DState.swapChain.currentBackBuffer = (D3DState.swapChain.currentBackBuffer + 1) % D3DState.swapChain.bufferCount;

    // Flush command queue
    D3DState.fence.currentFence++;

    D3DState.device.commandQueue->Signal(D3DState.fence.handle.Get(), D3DState.fence.currentFence);

    if(D3DState.fence.handle->GetCompletedValue() < D3DState.fence.currentFence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

        D3DState.fence.handle->SetEventOnCompletion(D3DState.fence.currentFence, eventHandle);

        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

void Run()
{
    while (d3dApp.running)
    {
        MSG message = {};
        if(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        else
        {
            DrawFrame();
        }
    }
}

int CALLBACK
WinMain(HINSTANCE hInstance, 
        HINSTANCE prevInstance,
		LPSTR lpCmdLine, 
        int showCmd)
{

    if(!InitWindow())
        return false;

    InitDirect3D();

    // run the application
    // main loop
    d3dApp.running = true;

    Run();

    return 0;
}