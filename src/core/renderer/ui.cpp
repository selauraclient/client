#include "ui.hpp"

static bool is_hovered(glm::vec2 mouse, glm::vec2 pos, glm::vec2 size) {
    return mouse.x >= pos.x && mouse.x <= pos.x + size.x &&
           mouse.y >= pos.y && mouse.y <= pos.y + size.y;
}

namespace sgfx::ui {
    void input_handler(const selaura::input_event& ev) {
        sgfx::ui::current_input = ev;
    }

    void button(glm::vec2 pos, glm::vec2 size, const std::string& text, float font_size, std::function<void()> callback) {
        glm::vec2 text_size = sgfx::get_text_size(text, font_size);

        sgfx::draw_rect(pos.x, pos.y, size.x, size.y, { 0.12f, 0.12f, 0.12f, 0.95f }, {10.f, 10.f, 10.f, 10.f});
        sgfx::draw_rect_stroke(pos.x, pos.y, size.x, size.y, 1.5f, {0.3f, 0.3f, 0.3f, 1.0f}, {10.f, 10.f, 10.f, 10.f});

        auto text_x = pos.x + (size.x - text_size.x) * 0.5f;
        auto text_y = pos.y + (size.y - text_size.y) * 0.5f;

        sgfx::draw_text(text, text_x, text_y, font_size, { 1.f, 1.f, 1.f, 1.f });
        if (current_input.is_button_down(selaura::mouse_button::left) && is_hovered({current_input.rendered_mouse_pos.x, current_input.rendered_mouse_pos.y}, pos, size)) {
            callback();
        }
    }
};