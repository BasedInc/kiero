<h1 align="center">kiero</h1>
<p align="center">
  <a href="https://github.com/Rebzzel/kiero/blob/master/LICENSE">
    <img src="https://img.shields.io/github/license/Rebzzel/kiero.svg?style=flat-square"/>
  </a>
  <a href="https://github.com/Rebzzel/kiero/stargazers">
    <img src="https://img.shields.io/github/stars/Rebzzel/kiero.svg?style=flat-square"/>
  </a>
  <br>
  Universal graphical hook for Direct3D, OpenGL, and Vulkan based games
</p>

### Requirement
[Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/) (Direct3D/OpenGL)

[Vulkan SDK](https://vulkan.lunarg.com/sdk/home)

### Usage
```C++
// Example for D3D9 hook

// Include required libraries
#include <d3d9.h>
#include <Windows.h>

#include <kiero.hpp>

// Create the type of function that we will hook
using pfnEndScene = HRESULT(__stdcall*)(LPDIRECT3DDEVICE9);
static pfnEndScene oEndScene = nullptr;

// Declare the detour function
static HRESULT __stdcall hk_EndScene(LPDIRECT3DDEVICE9 pDevice) {
    // static bool init = false;
    // if (!init) {
    //     MessageBox(0, "Hello, World!", "Kiero", MB_OK);
    //     init = true;
    // }
    return oEndScene(pDevice);
}

static DWORD WINAPI kieroExampleThread(LPVOID) {
    // Explicitly initialize for D3D9, "RenderType::Auto" can also be used
    if (kiero::init(kiero::RenderType::D3D9) != kiero::Status::Success) {
        return S_FALSE;
    }

    // Using kiero::bind to hook EndScene
    kiero::Status status = kiero::bind<&IDirect3DDevice9::EndScene>(&oEndScene, &hk_EndScene);

    if (status == kiero::Status::Success) {
        std::println("Successfully hooked IDirect3DDevice9::EndScene");
    }

    // Alternatively, get the address of the function via kiero::getMethod.
    // By default, the return type is the same as the expected detour function type
    pfnEndScene endScene = kiero::getMethod<&IDirect3DDevice9::EndScene>();

    // A template parameter can be used to specify the return type to anything
    // which can be bit_cast-ed from a uintptr_t
    uintptr_t endScene = kiero::getMethod<&IDirect3DDevice9::EndScene, uintptr_t>();

    return S_OK;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        auto handle = CreateThread(nullptr, 0, &kieroExampleThread, nullptr, 0, nullptr);
        if (!handle) {
            return FALSE;
        }
        CloseHandle(handle);
    }
    return TRUE;
}
```
