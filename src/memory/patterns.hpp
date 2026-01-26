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
            static uintptr_t cached = [] {
                uintptr_t addr =
                    reinterpret_cast<uintptr_t>(
                        hat::find_pattern(compiled_pattern, ".text").get()
                    );

                if (!addr) return uintptr_t{0};

                if constexpr (!std::is_void_v<Offset>) {
                    const uintptr_t target_ptr = addr + Offset::offset;

                    if constexpr (std::is_same_v<typename Offset::mode, deref>) {
                        std::int32_t disp{};
                        std::memcpy(&disp, (void*)target_ptr, sizeof(disp));
                        return target_ptr + sizeof(disp) + disp;
                    } else {
                        std::int32_t val{};
                        std::memcpy(&val, (void*)target_ptr, sizeof(val));
                        return static_cast<uintptr_t>(val);
                    }
                }

                return addr;
            }();

            return cached;
        }

        template <typename T>
            requires std::is_pointer_v<T>
        static T resolve() {
            return reinterpret_cast<T>(resolve());
        }
    };
};