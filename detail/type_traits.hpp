#pragma once
#include <pch.hpp>

namespace Selaura::Detail {
    template <typename T>
    struct MPF_Traits;

    template <typename Ret, typename Base, typename... Args>
    struct MPF_Traits<Ret(Base::*)(Args...)> {
        using Return_Type = Ret;
        using Base_Type = Base;
        using Args_Tuple = std::tuple<Args...>;
        static constexpr bool is_const = false;
    };

    template <typename Ret, typename Base, typename... Args>
    struct MPF_Traits<Ret(Base::*)(Args...) const> {
        using Return_Type = Ret;
        using Base_Type = Base;
        using Args_Tuple = std::tuple<Args...>;
        static constexpr bool is_const = true;
    };

};