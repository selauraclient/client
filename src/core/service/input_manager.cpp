#include "input_manager.hpp"

#include "core/service_manager.hpp"
#include "core/event/event.hpp"
#include "core/event/event_manager.hpp"

namespace selaura {
    void input_manager::update(UINT uMsg, WPARAM wParam, LPARAM lParam) {
        auto vk = static_cast<uint32_t>(wParam);

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

        selaura::input_event ev{};
        ev.keys_curr = this->keys_curr;
        ev.keys_prev = this->keys_prev;

        selaura::get<selaura::event_manager>().dispatch(ev);
    }

    void input_manager::update(GameInputMouseState state) {
        glm::vec2 current_abs_pos = { (float)state.absolutePositionX, (float)state.absolutePositionY };

        if (!this->cancel_input && was_cancelled) {
            this->last_mouse_pos = current_abs_pos;
            this->mouse_pos = current_abs_pos;
            this->mouse_delta = { 0.f, 0.f };
            was_cancelled = false;
        }

        if (this->cancel_input) {
            this->last_mouse_pos = current_abs_pos;
            this->mouse_delta = { 0.f, 0.f };
            this->scroll_wheel_delta = 0;
            was_cancelled = true;
        } else {
            this->mouse_delta = current_abs_pos - this->last_mouse_pos;
            this->mouse_pos = current_abs_pos;
            this->scroll_wheel_delta = state.wheelY;
            this->last_mouse_pos = current_abs_pos;
            was_cancelled = false;
        }

        previous_buttons = current_buttons;
        current_buttons = state.buttons;

        selaura::input_event ev{};
        ev.mouse_pos = this->mouse_pos;
        ev.mouse_delta = this->mouse_delta;
        ev.scroll_wheel_delta = this->scroll_wheel_delta;
        selaura::get<selaura::event_manager>().dispatch(ev);
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