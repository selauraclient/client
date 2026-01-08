#include "input_manager.hpp"

namespace selaura {
    void input_manager::update(UINT uMsg, WPARAM wParam, LPARAM lParam) {
        uint32_t vk = static_cast<uint32_t>(wParam);

        if (vk == VK_SHIFT) {
            uint32_t scancode = (lParam & 0x00ff0000) >> 16;
            vk = static_cast<uint32_t>(MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK_EX));
        }
        else if (vk == VK_CONTROL) {
            vk = (lParam & 0x01000000) ? VK_RCONTROL : VK_LCONTROL;
        }
        else if (vk == VK_MENU) {
            vk = (lParam & 0x01000000) ? VK_RMENU : VK_LMENU;
        }

        if (vk >= 256) return;

        if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) {
            keys_curr.set(vk, true);
        }
        else if (uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP) {
            keys_curr.set(vk, false);
        }
    }

    void input_manager::update(GameInputMouseState state) {
        bool has_movement = (state.positionX != 0 || state.positionY != 0);
        bool has_abs_pos = (state.absolutePositionX != 0 || state.absolutePositionY != 0);
        bool has_buttons = (state.buttons != GameInputMouseButtons::GameInputMouseNone);

        if (has_movement || has_abs_pos || has_buttons) {
            bool now_cancelled = this->cancel_input;

            if (now_cancelled != was_cancelled) {
                this->mouse_pos = {
                    state.absolutePositionX,
                    state.absolutePositionY
                };

                this->last_mouse_pos = this->mouse_pos;
                this->mouse_delta = { 0.f, 0.f };
                this->scroll_wheel_delta = 0;

                previous_buttons = current_buttons;
                was_cancelled = now_cancelled;
                return;
            }

            if (now_cancelled) {
                this->mouse_delta = { 0.f, 0.f };
                this->scroll_wheel_delta = 0;
                return;
            }

            previous_buttons = current_buttons;
            current_buttons = state.buttons;

            this->mouse_pos = {
                state.absolutePositionX,
                state.absolutePositionY
            };

            this->last_mouse_pos = this->mouse_pos;

            this->mouse_delta = {
                state.positionX,
                state.positionY
            };

            this->scroll_wheel_delta = state.wheelY;
        }
    }


    bool input_manager::is_down(uint32_t button) const {
        return (static_cast<uint32_t>(current_buttons) & static_cast<uint32_t>(button)) != 0;
    }

    bool input_manager::is_pressed(uint32_t button) const {
        bool now = (static_cast<uint32_t>(current_buttons) & static_cast<uint32_t>(button)) != 0;
        bool prev = (static_cast<uint32_t>(previous_buttons) & static_cast<uint32_t>(button)) != 0;
        return now && !prev;
    }

    glm::vec2 input_manager::get_mouse_pos() const {
        return this->mouse_pos;
    }

    glm::vec2 input_manager::get_mouse_delta() const {
        return this->mouse_delta;
    }

    int32_t input_manager::get_scroll_delta() const {
        return this->scroll_wheel_delta;
    }

    bool input_manager::is_key_down(uint32_t vk) const {
        return keys_curr.test(vk);
    }

    bool input_manager::is_key_pressed(uint32_t vk) const {
        return keys_curr.test(vk) && !keys_prev.test(vk);
    }

    void input_manager::set_input_cancelled(bool state) {
        this->cancel_input = state;
    }

    bool input_manager::is_input_cancelled() const {
        return this->cancel_input;
    }

    glm::vec2 input_manager::get_last_mouse_pos() const {
        return this->last_mouse_pos;
    }
};