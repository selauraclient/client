#pragma once
#include <pch.hpp>

namespace selaura {

    template <auto T>
    struct hook_storage {
        static inline safetyhook::InlineHook m_hook;
    };

    template <auto T, auto detour = nullptr>
    class hook {
    public:
        static void enable() {
            enable_internal(get_target_address());
        }

        template<typename InstanceType>
        static void enable(InstanceType* instance) {
            check_detour();

            void* target = nullptr;

            if constexpr (std::is_member_function_pointer_v<decltype(T)>) {
                constexpr size_t idx = selaura::vtable_index<T>();
                void** vtable = *reinterpret_cast<void***>(instance);
                target = vtable[idx];
            } else {
                target = reinterpret_cast<void*>(T);
            }

            enable_internal(target);
        }

        static void disable() {
            hook_storage<T>::m_hook = {};
        }

        template <typename... Args>
        static auto call(Args&&... args) {
            using FnType = decltype(T);

            if constexpr (std::is_member_function_pointer_v<FnType>) {
                return hook_storage<T>::m_hook
                    .template call<typename selaura::fn_traits<FnType>::ret>(
                        std::forward<Args>(args)...);
            } else {
                return hook_storage<T>::m_hook
                    .template call<Args...>(std::forward<Args>(args)...);
            }
        }

    private:
        static constexpr auto resolved_detour() {
            if constexpr (detour != nullptr) {
                return detour;
            } else {
                return &selaura::detour<T>::hk;
            }
        }

        static void check_detour() {
            if constexpr (detour == nullptr) {
                static_assert(
                    selaura::detail::has_auto_detour<T>,
                    "selaura::hook<T>: no detour provided and "
                    "no selaura::detour<T>::hk found"
                );
            }
        }

        static void enable_internal(void* target) {
            check_detour();

            auto& state = hook_storage<T>::m_hook;
            if (state || !target) return;

            auto builder =
                safetyhook::create_inline(target, resolved_detour());

            if (builder) {
                state = std::move(builder);
                detail::register_for_cleanup(
                    [] { hook<T, detour>::disable(); });
            }
        }

        static void* get_target_address() {
            if constexpr (std::is_member_function_pointer_v<decltype(T)>) {
                union { decltype(T) fn; void* ptr; } u;
                u.fn = T;
                return u.ptr;
            } else {
                return reinterpret_cast<void*>(T);
            }
        }
    };

};