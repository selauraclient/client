#include "hud_editor.hpp"

#include "core/service_manager.hpp"
#include "core/event/event_manager.hpp"
#include "core/renderer/sgfx.hpp"
#include "core/renderer/ui.hpp"

namespace selaura {
    screen_type hud_editor::get_type() {
        return screen_type::hud_editor;
    }

    void hud_editor::on_enable() {
        selaura::get<selaura::event_manager>().subscribe(this, &hud_editor::on_render);
        selaura::get<selaura::event_manager>().subscribe(this, &hud_editor::on_input);
    }

    void hud_editor::on_disable() {
        selaura::get<selaura::event_manager>().unsubscribe(this, &hud_editor::on_render);
        selaura::get<selaura::event_manager>().unsubscribe(this, &hud_editor::on_input);
    }

    inline bool is_hovered(float mx, float my, float x, float y, float w, float h) {
        return mx >= x && mx <= x + w && my >= y && my <= y + h;
    }

    void hud_editor::on_render(render_event& ev) {
        sgfx::draw_blur(0, 0, ev.screen_width, ev.screen_height, 2.f, 4);

        std::string label = "Open Settings";
        float font_size = 20.f;
        glm::vec2 padding = { 20.f, 10.f };

        glm::vec2 ts = sgfx::get_text_size(label, font_size);
        float btn_w = ts.x + (padding.x * 2);
        float btn_h = ts.y + (padding.y * 2);

        float x = (ev.screen_width - btn_w) / 2.f;
        float y = (ev.screen_height - btn_h) / 2.f;

        sgfx::ui::button({x, y}, {btn_w, btn_h}, "Open Settings", 20.f, []() {
            auto& scrn = selaura::get<selaura::screen_manager>();
            scrn.disable_screen<selaura::hud_editor>();
            scrn.enable_screen<selaura::clickgui>();
        });
    }

    void hud_editor::on_input(input_event& ev) {

    }
};