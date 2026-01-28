#pragma once
#include <pch.hpp>
#include <sdk/bedrock/core/GameInput_GDK.h>

using namespace GameInput::v2;
namespace selaura {
    struct input_manager {
        void update(UINT uMsg, WPARAM wParam, LPARAM lParam);
        void update(GameInputMouseState state);

        [[nodiscard]] glm::vec2 get_mouse_pos() const;
        [[nodiscard]] glm::vec2 get_mouse_delta() const;
        [[nodiscard]] glm::vec2 get_last_mouse_pos() const;

        [[nodiscard]] int32_t get_scroll_delta() const;

        [[nodiscard]] bool is_key_down(uint32_t vk) const;
        [[nodiscard]] bool is_key_pressed(uint32_t vk) const;

        [[nodiscard]] bool is_down(uint32_t button) const;
        [[nodiscard]] bool is_pressed(uint32_t button) const;

        void set_input_cancelled(bool state);
        [[nodiscard]] bool is_input_cancelled() const;
    private:
        std::bitset<256> keys_curr{};
        std::bitset<256> keys_prev{};

        glm::vec2 mouse_pos{};
        glm::vec2 last_mouse_pos{};
        glm::vec2 mouse_delta{};
        int32_t scroll_wheel_delta{ 0 };
        bool cancel_input{ false };
        bool was_cancelled{ false };

        GameInputMouseButtons current_buttons{ GameInputMouseButtons::GameInputMouseNone };
        GameInputMouseButtons previous_buttons{ GameInputMouseButtons::GameInputMouseNone };
    };
};