#pragma once
#include <pch.hpp>

#include "../screen.hpp"

namespace selaura {
    struct clickgui : screen {
        clickgui() {
            set_key(0xA1);
        }

        void on_enable() override;
        void on_disable() override;

        void on_render(render_event& ev) override;
        void on_mcupdate(mcgame_update& ev) override {};
        void on_input(input_event& ev) override;

        screen_type get_type() override;
    };
};