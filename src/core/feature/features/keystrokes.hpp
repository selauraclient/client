#pragma once
#include <pch.hpp>

#include "../feature.hpp"

namespace selaura {
    struct keystrokes : feature {
        keystrokes() {}

        void on_enable() override;
        void on_disable() override;

        void on_render(render_event& ev);
        void on_mcupdate(mcgame_update& ev);

        std::string_view get_name() const override { return name; }
        static constexpr hat::fixed_string name = "Keystrokes";
    private:
        bool up{}, left{}, right{}, down{}, jump{};

        float up_t   = 0.f;
        float left_t = 0.f;
        float right_t= 0.f;
        float down_t = 0.f;
        float jump_t = 0.f;
    };
};