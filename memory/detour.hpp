#pragma once
#include <pch.hpp>

namespace selaura {

    template <auto T>
    struct detour;

    namespace detail {
        template <auto T, auto Explicit>
        struct detour_resolver {
            static constexpr auto value = Explicit;
        };

        template <auto T>
        struct detour_resolver<T, nullptr> {
            static constexpr auto value = &selaura::detour<T>::hk;
        };

        template <auto T>
        concept has_auto_detour = requires { &selaura::detour<T>::hk; };
    };
};