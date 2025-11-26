#pragma once

#include "kiero_d3dcommon.hpp"

#include <d3d9.h>
#include <winrt/base.h>

namespace kiero {

    template<>
    inline Status initRenderType<RenderType::D3D9>() {
        using PFN_DIRECT3D_CREATE9 = decltype(&Direct3DCreate9);

        const HMODULE libD3D9 = ::GetModuleHandle(_TEXT("d3d9.dll"));
        if (libD3D9 == nullptr) {
            return Status::ModuleNotFoundError;
        }

        const auto Direct3DCreate9 = reinterpret_cast<PFN_DIRECT3D_CREATE9>(
            GetProcAddress(libD3D9, "Direct3DCreate9"));
        if (Direct3DCreate9 == nullptr) {
            return Status::UnknownError;
        }

        winrt::com_ptr<IDirect3D9> direct3D9{Direct3DCreate9(D3D_SDK_VERSION), winrt::take_ownership_from_abi};
        if (direct3D9 == nullptr) {
            return Status::UnknownError;
        }

        const auto window = d3d::createTempWindow();

        D3DPRESENT_PARAMETERS params{
            .BackBufferWidth = 0,
            .BackBufferHeight = 0,
            .BackBufferFormat = D3DFMT_UNKNOWN,
            .BackBufferCount = 0,
            .MultiSampleType = D3DMULTISAMPLE_NONE,
            .MultiSampleQuality = NULL,
            .SwapEffect = D3DSWAPEFFECT_DISCARD,
            .hDeviceWindow = static_cast<HWND>(window),
            .Windowed = 1,
            .EnableAutoDepthStencil = 0,
            .AutoDepthStencilFormat = D3DFMT_UNKNOWN,
            .Flags = NULL,
            .FullScreen_RefreshRateInHz = 0,
            .PresentationInterval = 0,
        };

        winrt::com_ptr<IDirect3DDevice9> device;
        const auto result = direct3D9->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_NULLREF,
            params.hDeviceWindow,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
            &params,
            device.put()
        );
        if (FAILED(result)) {
            return Status::UnknownError;
        }

        add_vtable(direct3D9);
        add_vtable(device);

        return Status::Success;
    }
}
