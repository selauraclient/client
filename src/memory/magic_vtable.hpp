#pragma once
#include <sdk/core/GameInput_GDK.h>

#ifndef MAGIC_VTABLE_LIB
#define MAGIC_VTABLE_LIB

#if defined(_MSC_VER)
    // Prior to Clang 19.1.0, mangling of functions with NTTP member pointers was incompatible with MSVC 19.20+:
    // https://releases.llvm.org/19.1.0/tools/clang/docs/ReleaseNotes.html#abi-changes-in-this-version
    #if (_MSC_VER < 1920) || (defined(__clang__) && __clang_major__ < 19)
        #define MAGIC_VTABLE_PREFIX std::string_view{"@@$"}
        #define MAGIC_VTABLE_SUFFIX std::string_view{"@@"}
    #else
        #define MAGIC_VTABLE_PREFIX std::string_view{"1@$"}
        #define MAGIC_VTABLE_SUFFIX std::string_view{"@@"}
    #endif
#else
    #error "Unsupported compiler"
#endif

#include <string_view>
#include <utility>

namespace magic_vft
{
    namespace detail
    {
        consteval uint8_t parse_alphabet_encoded_hex(const char ch)
        {
            if (ch < 'A' || ch >= 'A' + 16)
            {
                std::unreachable();
            }
            return static_cast<uint8_t>(ch - 'A');
        }

        consteval size_t decode_microsoft_value(std::string_view str)
        {
            // weird cases that i'm not sure how to handle
            if (str.starts_with("B3A"))
                return 4;
            if (str.starts_with("B7A"))
                return 8;

            // Skip the leading 'B'
            if (str.front() != 'B')
            {
                // invoke UB to stop constexpr evaluation
                std::unreachable();
            }
            str.remove_prefix(1);

            size_t value{};
            while (!str.empty() && str.front() != '@')
            {
                value *= 16;
                value += parse_alphabet_encoded_hex(str.front());
                str.remove_prefix(1);
            }
            return value;
        }
    }

    template<auto>
    consteval size_t vtable_index()
    {
        constexpr std::string_view mangled{__FUNCDNAME__};
        constexpr auto first = mangled.find(MAGIC_VTABLE_PREFIX) + MAGIC_VTABLE_PREFIX.size();
        constexpr auto last = mangled.find(MAGIC_VTABLE_SUFFIX, first);
        constexpr auto value = detail::decode_microsoft_value(mangled.substr(first, last - first));
        return value / sizeof(size_t);
    }

}

#endif // MAGIC_VTABLE_LIB