#pragma once

namespace kiero::detail {

    template<typename T>
    struct member_pointer_traits;

    template<typename Ret, typename Base, typename... Args>
    struct member_pointer_traits<Ret(Base::*)(Args...)> {
        using base_type = Base;
        // For COM ABI, this should always be compatible with the member function pointer type
        using detour_type = Ret(*)(Base*, Args...);
    };

    template<typename Ret, typename Base, typename... Args>
    struct member_pointer_traits<Ret(Base::*)(Args...) noexcept>
        : member_pointer_traits<Ret(Base::*)(Args...)> {};

    template<typename Ret, typename Base, typename... Args>
    struct member_pointer_traits<Ret(* Base::*)(Args...)> {
        using base_type = Base;
        using detour_type = Ret(*)(Args...);
    };
}
