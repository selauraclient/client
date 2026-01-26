#pragma once
#include <pch.hpp>

#include "features/coords.hpp"
#include "features/keystrokes.hpp"
#include "features/third_person_nametag.hpp"

namespace selaura {
    struct feature_manager {
        template <typename T>
        void enable_feature() {
            auto& feature = std::get<T>(features);
            feature.set_enabled(true);
        }

        template <typename T>
        void disable_feature() {
            auto& feature = std::get<T>(features);
            feature.set_enabled(false);
        }

        template <typename F>
        void for_each(F&& callback) {
            std::apply(
                [&](auto&... args) {
                    (std::forward<F>(callback)(args), ...);
                },
                features
            );
        }
    private:
        std::tuple<coords, keystrokes, third_person_nametag> features{};
    };
};