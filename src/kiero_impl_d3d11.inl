#pragma once

#include "kiero_d3dcommon.hpp"

#include <d3d11_4.h>
#include <dxgi1_5.h>
#include <winrt/base.h>

namespace kiero {

    template<>
    inline Status initRenderType<RenderType::D3D11>() {
        const HMODULE libD3D11 = ::GetModuleHandle(_T("d3d11.dll"));
        if (libD3D11 == nullptr) {
            return Status::ModuleNotFoundError;
        }

        const auto D3D11CreateDeviceAndSwapChain = reinterpret_cast<PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN>(
            GetProcAddress(libD3D11, "D3D11CreateDeviceAndSwapChain"));
        if (D3D11CreateDeviceAndSwapChain == nullptr) {
            return Status::UnknownError;
        }

        D3D_FEATURE_LEVEL featureLevel;
        constexpr D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0 };

        const auto window = d3d::createTempWindow();
        const DXGI_SWAP_CHAIN_DESC swapChainDesc = d3d::createSwapChainDesc(static_cast<HWND>(window));

        winrt::com_ptr<IDXGISwapChain> swapChain;
        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> context;

        const auto result = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            featureLevels,
            2,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            swapChain.put(),
            device.put(),
            &featureLevel,
            context.put()
        );
        if (FAILED(result)) {
            return Status::UnknownError;
        }

        add_vtable<IDXGISwapChain1, IDXGISwapChain2, IDXGISwapChain3, IDXGISwapChain4>(swapChain);
        add_vtable<ID3D11Device1, ID3D11Device2, ID3D11Device3, ID3D11Device4, ID3D11Device5>(device);
        add_vtable<ID3D11DeviceContext1, ID3D11DeviceContext2, ID3D11DeviceContext3, ID3D11DeviceContext4>(context);

        return Status::Success;
    }
}
