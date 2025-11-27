#include <kiero.hpp>

#include "impl/renderer_impl.h"

#ifndef KIERO_USE_MINHOOK
# error "The example requires that minhook be enabled!"
#endif

#include <Windows.h>

static DWORD WINAPI kieroExampleThread(LPVOID) {
    if (kiero::init(kiero::RenderType::Auto) != kiero::Status::Success) {
        return FALSE;
    }

    switch (kiero::getRenderType()) {
        using enum kiero::RenderType;
        case D3D9:
            impl::d3d9::init();
            break;
        case D3D10:
            impl::d3d10::init();
            break;
        case D3D11:
            impl::d3d11::init();
            break;
        case D3D12:
            impl::d3d12::init();
            break;
        case OpenGL:
            impl::opengl::init();
            break;
        case Vulkan:
            // TODO: Vulkan implementation?
        case None:
        case Auto:
            break;
    }
    return TRUE;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInstance);
        CreateThread(nullptr, 0, &kieroExampleThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
