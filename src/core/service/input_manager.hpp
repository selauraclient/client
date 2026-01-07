#pragma once
#include <pch.hpp>

using namespace GameInput::v2;
namespace selaura {

    enum mouse_button : uint32_t {
        left = 1,
        right = 2,
        middle = 4,
        button4 = 8,
        button5 = 16,
        wheel_tilt_left = 32,
        wheel_tilt_right = 64
    };

    struct input_manager {
        void update(UINT uMsg, WPARAM wParam, LPARAM lParam);
        void update(GameInputMouseState state);

        glm::vec2 get_mouse_pos() const;
        glm::vec2 get_mouse_delta() const;
        int32_t get_scroll_delta() const;

        bool is_key_down(uint32_t vk) const;
        bool is_key_pressed(uint32_t vk) const;

        bool is_down(uint32_t button) const;
        bool is_pressed(uint32_t button) const;
    private:
        std::bitset<256> keys_curr{};
        std::bitset<256> keys_prev{};

        glm::vec2 mouse_pos{};
        glm::vec2 mouse_delta{};
        int32_t scroll_wheel_delta{ 0 };

        GameInputMouseButtons current_buttons{ GameInputMouseButtons::GameInputMouseNone };
        GameInputMouseButtons previous_buttons{ GameInputMouseButtons::GameInputMouseNone };
    };
};