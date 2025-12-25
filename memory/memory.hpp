#pragma once
#include <pch.hpp>

namespace selaura {
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