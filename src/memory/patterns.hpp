#pragma once
#include <pch.hpp>

namespace selaura {
    struct ref {};
    struct deref {};

    template<typename Mode, std::size_t OffsetValue>
    struct signature_offset {
        using mode = Mode;
        static constexpr std::size_t offset = OffsetValue;
    };

    template <hat::fixed_string str, typename Offset = void>
    struct pattern {
        static constexpr auto compiled_pattern = hat::compile_signature<str>();
        using offset_type = Offset;

        static uintptr_t resolve() {
            uintptr_t addr = reinterpret_cast<uintptr_t>(hat::find_pattern(compiled_pattern, ".text").get());

            if constexpr (!std::is_void_v<Offset>) {
                if constexpr (std::is_same_v<typename Offset::mode, deref>) {
                    int32_t displacement = *reinterpret_cast<int32_t*>(addr + Offset::offset);
                    uintptr_t next_instruction = addr + Offset::offset + sizeof(int32_t);
                    return next_instruction + displacement;
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