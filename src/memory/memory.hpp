#pragma once
#include <pch.hpp>

namespace selaura {
    template<typename>
    struct fn_traits;

    template<typename R, typename... Args>
    struct fn_traits<R(*)(Args...)> {
        using ret = R;
        using fn  = R(*)(Args...);
    };

    template<typename R, typename C, typename... Args>
    struct fn_traits<R(C::*)(Args...)> {
        using ret = R;
        using fn  = R(*)(C*, Args...);
    };

    template<typename R, typename C, typename... Args>
    struct fn_traits<R(C::*)(Args...) noexcept> : fn_traits<R(C::*)(Args...)> {};

    template<typename T>
    static uintptr_t mfp_to_addr(T mfp) {
        union { T f; uintptr_t a; } u;
        u.f = mfp;
        return u.a;
    }

    template <class T = void, class... Args>
    T call_vfunc(auto* thisptr, std::size_t offset, Args&&... args) {
        using this_ = std::remove_cvref_t<decltype(*thisptr)>;

        static_assert(std::is_class_v<this_>);
        union {
            void* ptr;
            T(__thiscall this_::* fn)(Args...);
        };
        static_assert(sizeof(fn) == 8);

        ptr = (*reinterpret_cast<void***>(thisptr))[offset];
        return (thisptr->*fn)(std::forward<Args>(args)...);
    }
};