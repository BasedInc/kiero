#pragma once

namespace kiero::detail {

    template<typename T>
    struct memfn_ptr_traits;

    template<typename Ret, typename Base, typename... Args>
    struct memfn_ptr_traits<Ret(Base::*)(Args...)> {
        using base_type = Base;
        // For COM ABI, this should always be compatible with the member function pointer type
        using detour_type = Ret(Base*, Args...);
    };

    template<typename Ret, typename Base, typename... Args>
    struct memfn_ptr_traits<Ret(Base::*)(Args...) noexcept>
        : memfn_ptr_traits<Ret(Base::*)(Args...)> {};
}
