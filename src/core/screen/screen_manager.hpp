#pragma once
#include <pch.hpp>

#include "screens/clickgui.hpp"
#include "screens/hud_editor.hpp"

namespace selaura {
    struct screen_manager {
        template <typename T>
        void enable_screen() {
            auto& screen = std::get<T>(screens);

            active_screen = screen;
            screen.set_enabled(true);
        }

        template <typename T>
        void disable_screen() {
            auto& screen = std::get<T>(screens);

            active_screen = std::nullopt;
            screen.set_enabled(false);
        }

        auto& get_active_screen()  {
            return active_screen;
        }

        bool any_screens_enabled() {
            return std::apply([](auto&... screen_args) {
                return (screen_args.get_enabled() || ...);
            }, screens);
        }

        template <typename F>
        void for_each(F&& callback) {
            std::apply(
                [&](auto&... args) {
                    (std::forward<F>(callback)(args), ...);
                },
                screens
            );
        }

        bool is_in_hud_screen = false;
    private:
        std::optional<std::reference_wrapper<screen>> active_screen;
        std::tuple<clickgui, hud_editor> screens{};
    };
};