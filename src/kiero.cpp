#include "kiero.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <tchar.h>
#include <cassert>

namespace kiero
{
    template<RenderType type> requires (type != RenderType::Auto && type != RenderType::None)
    static Status initRenderType()
    {
        return Status::NotSupportedError;
    }
}

static kiero::RenderType g_renderType = kiero::RenderType::None;
static uintptr_t* g_methodsTable = nullptr;

#ifdef KIERO_INCLUDE_D3D9
#include "kiero_impl_d3d9.hpp"
#endif

#ifdef KIERO_INCLUDE_D3D10
#include "kiero_impl_d3d10.hpp"
#endif

#ifdef KIERO_INCLUDE_D3D11
#include "kiero_impl_d3d11.hpp"
#endif

#ifdef KIERO_INCLUDE_D3D12
#include "kiero_impl_d3d12.hpp"
#endif

#ifdef KIERO_INCLUDE_OPENGL
#include "kiero_impl_opengl.hpp"
#endif

#ifdef KIERO_INCLUDE_VULKAN
#include "kiero_vulkan.hpp"
#endif

#ifdef KIERO_USE_MINHOOK
#include <MinHook.h>
#endif

kiero::Status kiero::init(RenderType renderType)
{
    if (g_renderType != RenderType::None)
    {
        return Status::AlreadyInitializedError;
    }

    if (renderType == RenderType::None)
    {
        return Status::Success;
    }

    if (renderType == RenderType::Auto)
    {
        if (::GetModuleHandle(_T("d3d9.dll")) != nullptr)
        {
            renderType = RenderType::D3D9;
        }
        else if (::GetModuleHandle(_T("d3d10.dll")) != nullptr)
        {
            renderType = RenderType::D3D10;
        }
        else if (::GetModuleHandle(_T("d3d11.dll")) != nullptr)
        {
            renderType = RenderType::D3D11;
        }
        else if (::GetModuleHandle(_T("d3d12.dll")) != nullptr)
        {
            renderType = RenderType::D3D12;
        }
        else if (::GetModuleHandle(_T("opengl32.dll")) != nullptr)
        {
            renderType = RenderType::OpenGL;
        }
        else if (::GetModuleHandle(_T("vulkan-1.dll")) != nullptr)
        {
            renderType = RenderType::Vulkan;
        }
        else
        {
            return Status::NotSupportedError;
        }
    }

    const auto status = [renderType]
    {
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
    if (status == Status::Success)
    {
#ifdef KIERO_USE_MINHOOK
        MH_Initialize();
#endif
        g_renderType = renderType;
    }
    return status;
}

void kiero::shutdown()
{
    if (g_renderType != RenderType::None)
    {
#ifdef KIERO_USE_MINHOOK
        MH_DisableHook(MH_ALL_HOOKS);
#endif

        ::free(g_methodsTable);
        g_methodsTable = nullptr;
        g_renderType = RenderType::None;
    }
}

kiero::Status kiero::bind([[maybe_unused]] uint16_t _index, void** _original, void* _function)
{
    // TODO: Need own detour function

    assert(_original != NULL && _function != NULL);

    if (g_renderType != RenderType::None)
    {
#ifdef KIERO_USE_MINHOOK
        const auto target = reinterpret_cast<void*>(g_methodsTable[_index]);
        if (MH_CreateHook(target, _function, _original) != MH_OK || MH_EnableHook(target) != MH_OK)
        {
            return Status::UnknownError;
        }
#endif

        return Status::Success;
    }

    return Status::NotInitializedError;
}

void kiero::unbind([[maybe_unused]] uint16_t _index)
{
    if (g_renderType != RenderType::None)
    {
#ifdef KIERO_USE_MINHOOK
        MH_DisableHook(reinterpret_cast<void*>(g_methodsTable[_index]));
#endif
    }
}

kiero::RenderType kiero::getRenderType()
{
    return g_renderType;
}

uintptr_t* kiero::getMethodsTable()
{
    return g_methodsTable;
} 