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
            if (!addr) return 0;

            if constexpr (!std::is_void_v<Offset>) {
                const uintptr_t target_ptr = addr + Offset::offset;

                if constexpr (std::is_same_v<typename Offset::mode, deref>) {
                    std::int32_t displacement{};
                    std::memcpy(&displacement, reinterpret_cast<void*>(target_ptr), sizeof(std::int32_t));

                    uintptr_t next_instruction = target_ptr + sizeof(std::int32_t);
                    return next_instruction + displacement;
                } else {
                    std::int32_t value{};
                    std::memcpy(&value, reinterpret_cast<void*>(target_ptr), sizeof(std::int32_t));
                    return static_cast<uintptr_t>(value);
                }
            }

            return addr;
        }
    };
};