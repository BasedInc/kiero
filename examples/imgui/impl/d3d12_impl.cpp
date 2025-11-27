#include "renderer_impl.h"

#ifdef KIERO_INCLUDE_D3D12

#include <cassert>
#include <dxgi1_4.h>
#include <d3d12.h>

#include <winrt/base.h>

#include "win32_impl.h"
#include "shared.h"

#include <kiero.hpp>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

namespace {

    // Largely based on the ImGui Win32 D3D12 example
    // https://github.com/ocornut/imgui/blob/master/examples/example_win32_directx12/main.cpp
    // Combines "APP_NUM_FRAMES_IN_FLIGHT" and "APP_NUM_BACK_BUFFERS"
    HWND g_Hwnd{};
    winrt::com_ptr<IDXGISwapChain3> g_pSwapChain{};
    winrt::com_ptr<ID3D12Device> g_pd3dDevice{};
    winrt::com_ptr<ID3D12CommandQueue> g_pd3dCommandQueue{};
    winrt::com_ptr<ID3D12GraphicsCommandList> g_pd3dCommandList{};
    winrt::com_ptr<ID3D12DescriptorHeap> g_pd3dRtvDescHeap{};
    winrt::com_ptr<ID3D12DescriptorHeap> g_pd3dSrvDescHeap{};

    struct FrameContext {
        winrt::com_ptr<ID3D12CommandAllocator> commandAllocator{};
        winrt::com_ptr<ID3D12Resource> resource{};
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
    };
    std::vector<FrameContext> g_frameContext{};

    void InitD3D(IDXGISwapChain* pSwapChain) {
        winrt::check_hresult(pSwapChain->QueryInterface(IID_PPV_ARGS(g_pSwapChain.put())));
        winrt::check_hresult(g_pSwapChain->GetDevice(IID_PPV_ARGS(g_pd3dDevice.put())));

        UINT framesInFlight;
        {
            DXGI_SWAP_CHAIN_DESC desc;
            winrt::check_hresult(g_pSwapChain->GetDesc(&desc));
            framesInFlight = desc.BufferCount;
            g_Hwnd = desc.OutputWindow;

            g_frameContext.clear();
            g_frameContext.resize(framesInFlight);
        }

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = framesInFlight;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;
            winrt::check_hresult(g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(g_pd3dRtvDescHeap.put())));

            const auto rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            auto rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < framesInFlight; i++) {
                g_frameContext[i].rtvHandle = rtvHandle;
                rtvHandle.ptr += rtvDescriptorSize;
            }
        }

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = 1;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            winrt::check_hresult(g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(g_pd3dSrvDescHeap.put())));
        }

        for (UINT i = 0; i < framesInFlight; i++) {
            winrt::check_hresult(g_pd3dDevice->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(g_frameContext[i].commandAllocator.put())
            ));
        }

        winrt::check_hresult(g_pd3dDevice->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            g_frameContext[0].commandAllocator.get(),
            nullptr,
            IID_PPV_ARGS(g_pd3dCommandList.put())
        ));
        winrt::check_hresult(g_pd3dCommandList->Close());
    }

    void CreateRenderTarget() {
        for (size_t i = 0; i < g_frameContext.size(); ++i) {
            auto& context = g_frameContext[i];
            winrt::com_ptr<ID3D12Resource> backBuffer;
            winrt::check_hresult(g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(backBuffer.put())));
            g_pd3dDevice->CreateRenderTargetView(backBuffer.get(), nullptr, context.rtvHandle);
            context.resource = std::move(backBuffer);
        }
    }

    void CleanupRenderTarget() {
        for (auto& context : g_frameContext) {
            context.resource = nullptr;
        }
    }
}

using Present = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
static Present oPresent{};

static HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, const UINT SyncInterval, const UINT Flags) {
    static bool init = false;

    if (!init) {
        if (g_pd3dCommandQueue == nullptr) {
            return oPresent(pSwapChain, SyncInterval, Flags);
        }

        InitD3D(pSwapChain);

        ImGui_ImplDX12_InitInfo init_info = {};
        init_info.Device = g_pd3dDevice.get();
        init_info.CommandQueue = g_pd3dCommandQueue.get();
        init_info.NumFramesInFlight = static_cast<int>(g_frameContext.size());
        init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

        // For simplicity, this is the legacy (pre-ImGui 1.91.6) approach that only supports one texture
        init_info.SrvDescriptorHeap = g_pd3dSrvDescHeap.get();
        init_info.LegacySingleSrvCpuDescriptor = g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
        init_info.LegacySingleSrvGpuDescriptor = g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();

        CreateRenderTarget();

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(g_Hwnd);
        ImGui_ImplDX12_Init(&init_info);

        impl::win32::init(g_Hwnd);

        init = true;
    }

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    impl::showExampleWindow("D3D12");

    ImGui::EndFrame();
    ImGui::Render();

    const auto& context = g_frameContext[g_pSwapChain->GetCurrentBackBufferIndex()];
    winrt::check_hresult(context.commandAllocator->Reset());

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = context.resource.get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    winrt::check_hresult(g_pd3dCommandList->Reset(context.commandAllocator.get(), nullptr));
    g_pd3dCommandList->ResourceBarrier(1, &barrier);

    // Render Dear ImGui graphics
    const std::array targets{context.rtvHandle};
    g_pd3dCommandList->OMSetRenderTargets(targets.size(), targets.data(), FALSE, nullptr);
    const std::array heaps{g_pd3dSrvDescHeap.get()};
    g_pd3dCommandList->SetDescriptorHeaps(heaps.size(), heaps.data());
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList.get());
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    winrt::check_hresult(g_pd3dCommandList->Close());

    const std::array<ID3D12CommandList*, 1> lists{g_pd3dCommandList.get()};
    g_pd3dCommandQueue->ExecuteCommandLists(lists.size(), lists.data());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

using ResizeBuffers = HRESULT(*)(IDXGISwapChain*, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
ResizeBuffers oResizeBuffers{};

HRESULT hkResizeBuffers(
    IDXGISwapChain*   pSwapChain,
    const UINT        BufferCount,
    const UINT        Width,
    const UINT        Height,
    const DXGI_FORMAT NewFormat,
    const UINT        SwapChainFlags
) {
    CleanupRenderTarget();
    const auto result = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    CreateRenderTarget();
    return result;
}

using ResizeBuffers1 = HRESULT(*)(IDXGISwapChain3*, UINT, UINT, UINT, DXGI_FORMAT, UINT, const UINT*, IUnknown* const*);
ResizeBuffers1 oResizeBuffers1{};

HRESULT hkResizeBuffers1(
    IDXGISwapChain3*  pSwapChain,
    const UINT        BufferCount,
    const UINT        Width,
    const UINT        Height,
    const DXGI_FORMAT NewFormat,
    const UINT        SwapChainFlags,
    const UINT*       pCreationNodeMask,
    IUnknown* const*  ppPresentQueue
) {
    CleanupRenderTarget();
    const auto result = oResizeBuffers1(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
    CreateRenderTarget();
    return result;
}

using ExecuteCommandLists = void(*)(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList *const *ppCommandLists);
ExecuteCommandLists oExecuteCommandLists{};

static void hkExecuteCommandLists(ID3D12CommandQueue* queue, const UINT NumCommandLists, ID3D12CommandList *const *ppCommandLists) {
    if (!g_pd3dCommandQueue) {
        g_pd3dCommandQueue.copy_from(queue);
    }
    return oExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
}

void impl::d3d12::init() {
    auto status = kiero::bind<&IDXGISwapChain::Present>(&oPresent, &hkPresent);
    assert(status == kiero::Status::Success);

    status = kiero::bind<&IDXGISwapChain::ResizeBuffers>(&oResizeBuffers, &hkResizeBuffers);
    assert(status == kiero::Status::Success);

    status = kiero::bind<&IDXGISwapChain3::ResizeBuffers1>(&oResizeBuffers1, &hkResizeBuffers1);
    assert(status == kiero::Status::Success);

    status = kiero::bind<&ID3D12CommandQueue::ExecuteCommandLists>(&oExecuteCommandLists, hkExecuteCommandLists);
    assert(status == kiero::Status::Success);
}

#else

void impl::d3d12::init() {}

#endif // KIERO_INCLUDE_D3D12
