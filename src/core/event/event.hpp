#pragma once
#include <pch.hpp>

#include <sdk/core/MinecraftGame.hpp>
#include <sdk/renderer/ScreenView.hpp>

namespace selaura {
    struct event {
        virtual ~event() = default;
        bool cancelled = false;
    };

    struct mcgame_update final : event {
        MinecraftGame* mc;
    };

    struct render_event final : event {
        IDXGISwapChain* swap_chain;
        float screen_width;
        float screen_height;
        float fps;
    };

    enum mouse_button : uint32_t {
        left = 1,
        right = 2,
        middle = 4,
        button4 = 8,
        button5 = 16,
        wheel_tilt_left = 32,
        wheel_tilt_right = 64
    };

    struct input_event final : event {
        glm::vec2 rendered_mouse_pos;
        glm::vec2 mouse_pos;
        std::bitset<256> keys_curr;
        std::bitset<256> keys_prev;
        glm::vec2 mouse_delta;
        int32_t scroll_wheel_delta;

        uint32_t key;
        mouse_button buttons;
        bool is_down;
        bool is_game_input;

        bool is_button_down(mouse_button b) const {
            return (static_cast<uint32_t>(buttons) & static_cast<uint32_t>(b)) != 0;
        }
    };

    struct sv_render_event final : event {
        ScreenView* sv;
        std::string screen_name;
    };
};