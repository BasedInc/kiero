#pragma once

#include <bit>
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
        MethodAlreadyBoundError = -8,
        MethodNotBoundError = -9,

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

    template<auto T, typename Detour = typename detail::member_pointer_traits<decltype(T)>::detour_type>
    Status bind(std::type_identity_t<Detour**> original, std::type_identity_t<Detour*> detour);

    template<auto T>
    Status unbind();

    RenderType getRenderType();

    template<auto T>
    uintptr_t getMethod();
}

namespace kiero::detail {

    Status bind(void* target, void** original, void* detour);
    Status unbind(void* target);
    uintptr_t getMethod(const void* guid, size_t index);
    void* getMethodTable();

    template<RenderType type>
    struct MethodTable {
        static constexpr auto render_type = type;
    };
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
        using base_type = typename detail::member_pointer_traits<decltype(T)>::base_type;

        if constexpr (std::is_member_function_pointer_v<decltype(T)>) {
            return detail::getMethod(&__uuidof(base_type), detail::magic_vft::vtable_index<T>());
        } else if constexpr (std::is_member_pointer_v<decltype(T)>) {
            if (getRenderType() != base_type::render_type) {
                return 0;
            }
            auto* table = static_cast<base_type*>(detail::getMethodTable());
            if (!table) {
                return 0;
            }
            return std::bit_cast<uintptr_t>(table->*T);
        } else {
            static_assert(sizeof(T) == 0, "Invalid method type");
            return 0;
        }
    }
}
