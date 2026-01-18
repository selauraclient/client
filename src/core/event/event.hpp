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

    struct input_event final : event {
        glm::vec2 mouse_pos;
        std::bitset<256> keys_curr;
        std::bitset<256> keys_prev;
        glm::vec2 mouse_delta;
        int32_t scroll_wheel_delta;

        uint32_t key;
        bool is_down;
        bool is_game_input;
    };

    struct sv_render_event final : event {
        ScreenView* sv;
        std::string screen_name;
    };
};