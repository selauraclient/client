#pragma once
#include <pch.hpp>

namespace selaura {
    namespace detail {
        constexpr uint8_t parse_msvc_alphabet_hex(const char ch) {
            return static_cast<uint8_t>(ch - 'A');
        }

        constexpr size_t decode_microsoft_vtable_value(std::string_view str) {
            using namespace std::literals::string_view_literals;

            if (str.substr(0, 3) == "B3A"sv) return 4;
            if (str.substr(0, 3) == "B7A"sv) return 8;

            if (str.empty() || str.front() != 'B') return 0;

            str.remove_prefix(1);

            size_t value = 0;
            for (char ch : str) {
                if (ch == '@') break;
                value *= 16;
                value += parse_msvc_alphabet_hex(ch);
            }
            return value;
        }

#if (_MSC_VER < 1920) || (defined(__clang__) && __clang_major__ < 19)
        inline constexpr std::string_view msvc_vtable_prefix = "@@$";
#else
        inline constexpr std::string_view msvc_vtable_prefix = "1@$";
#endif
        inline constexpr std::string_view msvc_vtable_suffix = "@@";
    }

    template <auto T>
    constexpr std::string_view get_func_sig() {
        constexpr std::string_view sig = __FUNCSIG__;

        constexpr std::string_view prefix = "get_func_sig<";
        size_t start = sig.find(prefix);
        if (start == std::string_view::npos) return sig;
        start += prefix.length();

        size_t end = sig.rfind('>');
        if (end == std::string_view::npos) return sig;

        return sig.substr(start, end - start);
    }

    template <auto T>
    constexpr bool is_virtual() {
        return get_func_sig<T>().find("::`vcall'{") != std::string_view::npos;
    }

    template <auto T>
    constexpr std::string_view func_sig_v = get_func_sig<T>();

    template <auto T>
    constexpr bool is_virtual_v = is_virtual<T>();

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

    template<auto T>
    consteval size_t vtable_index() {
        constexpr std::string_view mangled{ __FUNCDNAME__ };

        constexpr auto first = mangled.rfind(detail::msvc_vtable_prefix) + detail::msvc_vtable_prefix.size();
        constexpr auto last = mangled.find(detail::msvc_vtable_suffix, first);
        constexpr auto value = detail::decode_microsoft_vtable_value(mangled.substr(first, last - first));

        return value / sizeof(size_t);
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