#pragma once

#include "kiero_d3dcommon.hpp"

#include <d3d10_1.h>
#include <d3d10.h>
#include <dxgi1_6.h>
#include <winrt/base.h>

namespace kiero {

    template<>
    inline Status initRenderType<RenderType::D3D10>() {
        using PFN_D3D10_CREATE_DEVICE_AND_SWAP_CHAIN = decltype(&::D3D10CreateDeviceAndSwapChain);

        const HMODULE libD3D10 = ::GetModuleHandle(_T("d3d10.dll"));
        if (libD3D10 == nullptr) {
            return Status::ModuleNotFoundError;
        }

        const auto D3D10CreateDeviceAndSwapChain = reinterpret_cast<PFN_D3D10_CREATE_DEVICE_AND_SWAP_CHAIN>(
            GetProcAddress(libD3D10, "D3D10CreateDeviceAndSwapChain"));
        if (D3D10CreateDeviceAndSwapChain == nullptr) {
            return Status::UnknownError;
        }

        const auto window = d3d::createTempWindow();
        DXGI_SWAP_CHAIN_DESC swapChainDesc = d3d::createSwapChainDesc(static_cast<HWND>(window));

        winrt::com_ptr<IDXGISwapChain> swapChain;
        winrt::com_ptr<ID3D10Device> device;

        const auto result = D3D10CreateDeviceAndSwapChain(
            nullptr,
            D3D10_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            D3D10_SDK_VERSION,
            &swapChainDesc,
            swapChain.put(),
            device.put()
        );
        if (FAILED(result)) {
            return Status::UnknownError;
        }

        add_vtable<IDXGISwapChain1, IDXGISwapChain2, IDXGISwapChain3, IDXGISwapChain4>(swapChain);
        add_vtable<ID3D10Device1>(device);

        return Status::Success;
    }
}
