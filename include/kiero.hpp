#pragma once

#include <cstdint>
#include <span>

#include <kiero/magic_vtable.hpp>
#include <kiero/type_traits.hpp>

namespace kiero {

    enum class Status {
        UnknownError = -1,
        NotSupportedError = -2,
        ModuleNotFoundError = -3,
        AlreadyInitializedError = -4,
        NotInitializedError = -5,
        NotImplementedError = -6,
        NoSuchInterfaceError = -7,
        MethodOutOfBoundsError = -8,

        Success = 0,
    };

    enum class RenderType {
        None,

        D3D9,
        D3D10,
        D3D11,
        D3D12,

        OpenGL,
        Vulkan,

        Auto
    };

    Status init(RenderType renderType);
    void shutdown();

    Status bind(uint16_t index, void** original, void* detour);
    Status unbind(uint16_t index);

    template<auto T, typename Detour = typename detail::memfn_ptr_traits<decltype(T)>::detour_type>
    Status bind(std::type_identity_t<Detour**> original, std::type_identity_t<Detour*> detour);

    template<auto T>
    Status unbind();

    RenderType getRenderType();
    std::span<const uintptr_t> getMethodsTable();

    template<auto T>
    uintptr_t getMethod();
}

typedef struct _GUID GUID;

namespace kiero::detail {

    Status bind(void* target, void** original, void* detour);
    Status unbind(void* target);
    uintptr_t getMethod(const GUID& guid, size_t index);
}

namespace kiero {

    template<auto T, typename Detour>
    Status bind(std::type_identity_t<Detour**> original, std::type_identity_t<Detour*> detour) {
        const auto method = getMethod<T>();
        if (!method) {
            return Status::NoSuchInterfaceError;
        }
        return detail::bind(reinterpret_cast<void*>(method), reinterpret_cast<void**>(original), reinterpret_cast<void*>(detour));
    }

    template<auto T>
    Status unbind() {
        const auto method = getMethod<T>();
        if (!method) {
            return Status::NoSuchInterfaceError;
        }
        return detail::unbind(reinterpret_cast<void*>(method));
    }

    template<auto T>
    uintptr_t getMethod() {
        using base_type = typename detail::memfn_ptr_traits<decltype(T)>::base_type;
        return detail::getMethod(__uuidof(base_type), detail::magic_vft::vtable_index<T>());
    }
}
