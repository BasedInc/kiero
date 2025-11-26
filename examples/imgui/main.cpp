#include <kiero.hpp>

#ifdef KIERO_INCLUDE_D3D9
# include "impl/d3d9_impl.h"
#endif

#ifdef KIERO_INCLUDE_D3D10
# include "impl/d3d10_impl.h"
#endif

#ifdef KIERO_INCLUDE_D3D11
# include "impl/d3d11_impl.h"
#endif

#ifdef KIERO_INCLUDE_D3D12
#endif

#ifdef KIERO_INCLUDE_OPENGL
#endif

#ifdef KIERO_INCLUDE_VULKAN
#endif

#ifndef KIERO_USE_MINHOOK
# error "The example requires that minhook be enabled!"
#endif

#include <Windows.h>

DWORD kieroExampleThread(LPVOID) {
    if (kiero::init(kiero::RenderType::Auto) == kiero::Status::Success) {
        switch (kiero::getRenderType()) {
            using enum kiero::RenderType;
            case D3D9:
#ifdef KIERO_INCLUDE_D3D9
                impl::d3d9::init();
#endif
                break;
            case D3D10:
#ifdef KIERO_INCLUDE_D3D10
                impl::d3d10::init();
#endif
                break;
            case D3D11:
#ifdef KIERO_INCLUDE_D3D11
                impl::d3d11::init();
#endif
                break;
            case D3D12:
                // TODO: D3D12 implementation?
            case OpenGL:
                // TODO: OpenGL implementation?
            case Vulkan:
                // TODO: Vulkan implementation?
            case None:
            case Auto:
                break;
        }

        return 1;
    }

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInstance);
        CreateThread(nullptr, 0, &kieroExampleThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
