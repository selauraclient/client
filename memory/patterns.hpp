//
// Created by chyves on 11/11/2025.
//

#pragma once
#include <pch.hpp>

namespace Selaura {
    struct Ref {};
    struct Deref {};

    template<typename Mode, std::size_t OffsetValue>
    struct SignatureOffset {
        using mode = Mode;
        static constexpr std::size_t offset = OffsetValue;
    };

    template <hat::fixed_string str, typename Offset = void>
    struct Pattern {
        static constexpr auto pattern = hat::compile_signature<str>();
        using offset_type = Offset;

        static uintptr_t resolve() {
            uintptr_t addr = reinterpret_cast<uintptr_t>(hat::find_pattern(pattern, ".text").get());

            if constexpr (!std::is_void_v<Offset>) {
                if constexpr (std::is_same_v<typename Offset::mode, Deref>) {
                    constexpr auto offset = *reinterpret_cast<int*>(addr + Offset::offset);
                    auto final = addr + offset + (static_cast<unsigned long long>(Offset::offset) + sizeof(int));
                    return reinterpret_cast<uintptr_t>(final);
                } else {
                    uintptr_t final = *reinterpret_cast<int *>(addr + Offset::offset);
                    return final;
                }
            } else {
                return addr;
            }
        }
    };
};