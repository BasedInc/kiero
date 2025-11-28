#include "kiero.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <tchar.h>

#include <unknwn.h>
#include <winrt/base.h>

#include <cassert>
#include <map>
#include <vector>

namespace kiero {

    template<RenderType type> requires (type != RenderType::Auto && type != RenderType::None)
    static Status initRenderType() {
        return Status::NotSupportedError;
    }
}

namespace {
    kiero::RenderType g_renderType = kiero::RenderType::None;
    std::vector<uintptr_t> g_methodsTable;
    std::map<winrt::guid, uintptr_t*> g_vTables;

    template<typename T> requires std::is_polymorphic_v<T>
    uintptr_t* vtable_for(const T& t) {
        return *reinterpret_cast<uintptr_t* const*>(&t);
    }

    template<typename... Impl, typename T> requires (std::derived_from<Impl, T> && ...)
    void add_vtable(const winrt::com_ptr<T>& t) {
        g_vTables.emplace(winrt::guid_of<T>(), vtable_for(*t));
        ([&] {
            if (auto impl = t.template try_as<Impl>(); impl) {
                g_vTables.emplace(winrt::guid_of<Impl>(), vtable_for(*impl));
            }
        }(), ...);
    }
}

#ifdef KIERO_INCLUDE_D3D9
#include "kiero_impl_d3d9.inl"
#endif

#ifdef KIERO_INCLUDE_D3D10
#include "kiero_impl_d3d10.inl"
#endif

#ifdef KIERO_INCLUDE_D3D11
#include "kiero_impl_d3d11.inl"
#endif

#ifdef KIERO_INCLUDE_D3D12
#include "kiero_impl_d3d12.inl"
#endif

#ifdef KIERO_INCLUDE_OPENGL
#include "kiero_impl_opengl.inl"
#endif

#ifdef KIERO_INCLUDE_VULKAN
#include "kiero_impl_vulkan.inl"
#endif

namespace kiero::hook {
    Status init();
    void shutdown();
    Status bind(void* target, void** original, void* detour);
    Status unbind(void* target);
}

kiero::Status kiero::init(RenderType renderType) {
    if (g_renderType != RenderType::None) {
        return Status::AlreadyInitializedError;
    }

    if (renderType == RenderType::None) {
        return Status::Success;
    }

    if (renderType == RenderType::Auto) {
        if (GetModuleHandle(_T("vulkan-1.dll")))
            renderType = RenderType::Vulkan;
        else if (GetModuleHandle(_T("opengl32.dll")))
            renderType = RenderType::OpenGL;
        else if (GetModuleHandle(_T("d3d12.dll")))
            renderType = RenderType::D3D12;
        else if (GetModuleHandle(_T("d3d11.dll")))
            renderType = RenderType::D3D11;
        else if (GetModuleHandle(_T("d3d10.dll")))
            renderType = RenderType::D3D10;
        else if (GetModuleHandle(_T("d3d9.dll")))
            renderType = RenderType::D3D9;
        else
            return Status::NotSupportedError;
    }

    const auto status = [renderType] {
        switch (renderType) {
            case RenderType::D3D9: return initRenderType<RenderType::D3D9>();
            case RenderType::D3D10: return initRenderType<RenderType::D3D10>();
            case RenderType::D3D11: return initRenderType<RenderType::D3D11>();
            case RenderType::D3D12: return initRenderType<RenderType::D3D12>();
            case RenderType::OpenGL: return initRenderType<RenderType::OpenGL>();
            case RenderType::Vulkan: return initRenderType<RenderType::Vulkan>();
            case RenderType::None: [[fallthrough]];
            case RenderType::Auto: break;
        }
        return Status::UnknownError;
    }();

    if (status == Status::Success) {
        hook::init();
        g_renderType = renderType;
    }
    return status;
}

void kiero::shutdown() {
    if (g_renderType == RenderType::None) {
        return;
    }
    hook::shutdown();
    g_methodsTable.clear();
    g_vTables.clear();
    g_renderType = RenderType::None;
}

kiero::Status kiero::bind(const uint16_t index, void** original, void* function) {
    if (index >= g_methodsTable.size()) {
        return Status::MethodOutOfBoundsError;
    }
    return detail::bind(reinterpret_cast<void*>(g_methodsTable[index]), original, function);
}

kiero::Status kiero::unbind([[maybe_unused]] const uint16_t index) {
    if (index >= g_methodsTable.size()) {
        return Status::MethodOutOfBoundsError;
    }
    detail::unbind(reinterpret_cast<void*>(g_methodsTable[index]));
    return Status::Success;
}

kiero::Status kiero::detail::bind(void* target, void** original, void* detour) {
    assert(target != nullptr && original != nullptr && detour != nullptr);

    if (g_renderType == RenderType::None) {
        return Status::NotInitializedError;
    }

    return hook::bind(target, original, detour);
}

kiero::Status kiero::detail::unbind(void* target) {
    assert(target != nullptr);

    if (g_renderType == RenderType::None) {
        return Status::NotInitializedError;
    }

    return hook::unbind(target);
}

uintptr_t kiero::detail::getMethod(const GUID& guid, const size_t index) {
    const auto it = g_vTables.find(winrt::guid{guid});
    if (it != g_vTables.end()) {
        return it->second[index];
    }
    return 0;
}

kiero::RenderType kiero::getRenderType() {
    return g_renderType;
}

std::span<const uintptr_t> kiero::getMethodsTable() {
    return g_methodsTable;
}
