#pragma once
#include <pch.hpp>

#include "core/event/event.hpp"

namespace selaura {
    enum class screen_type {
        clickgui
    };

    struct screen {
        virtual ~screen() = default;

        virtual void on_enable() = 0;
        virtual void on_disable() = 0;
        void set_enabled(bool state);
        bool get_enabled();

        void toggle();

        virtual void on_render(render_event& ev) = 0;
        virtual void on_mcupdate(mcgame_update& ev) = 0;
        virtual void on_input(input_event& ev) = 0;

        virtual screen_type get_type() = 0;

        void set_key(uint32_t new_key);
        uint32_t get_key();
    private:
        bool enabled = false;
        uint32_t key = 0;
    };
};