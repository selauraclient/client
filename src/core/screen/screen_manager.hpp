#pragma once
#include <pch.hpp>

#include "screens/clickgui.hpp"

namespace selaura {
    struct screen_manager {
        template <typename T>
        void enable_screen() {
            auto& screen = std::get<T>(screens);

            active_screen = screen;
            screen.set_enabled(true);
        }

        auto& get_active_screen()  {
            return active_screen;
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
    private:
        std::optional<std::reference_wrapper<screen>> active_screen;
        std::tuple<clickgui> screens{};
    };
};