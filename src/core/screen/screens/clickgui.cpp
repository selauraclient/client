#include "clickgui.hpp"

#include "core/service_manager.hpp"
#include "core/event/event_manager.hpp"
#include "core/renderer/sgfx.hpp"

namespace selaura {
    screen_type clickgui::get_type() {
        return screen_type::clickgui;
    }

    void clickgui::on_enable() {
        selaura::get<selaura::event_manager>().subscribe(this, &clickgui::on_render);
    }

    void clickgui::on_disable() {
        selaura::get<selaura::event_manager>().unsubscribe(this, &clickgui::on_render);
    }

    bool is_hovered(float mx, float my, float x, float y, float w, float h) {
        return mx >= x && mx <= x + w && my >= y && my <= y + h;
    }

    void clickgui::on_render(render_event& ev) {
        sgfx::draw_blur(0, 0, ev.screen_width, ev.screen_height, 2.f, 4);

        float menu_width = ev.screen_width * 0.55f;
        float menu_height = ev.screen_height * 0.65f;
        float menu_x = (ev.screen_width - menu_width) / 2;
        float menu_y = (ev.screen_height - menu_height) / 2;
        float sidebar_width = menu_width * 0.27f;

        sgfx::draw_rect(menu_x, menu_y, menu_width, menu_height, {34, 34, 34, 1.f}, {15, 15, 15, 15});
        sgfx::draw_rect(menu_x, menu_y, sidebar_width, menu_height, {24, 24, 24, 1.f}, {15, 0, 0, 15});

        const float padding = 15.f;
        const int cols = 4;
        float content_x = menu_x + sidebar_width + padding;
        float content_y = menu_y + padding;
        float content_width = menu_width - sidebar_width - (padding * 2);

        float card_width = (content_width - (padding * (cols - 1))) / cols;
        float card_height = card_width * 0.95f;

        auto& feature_manager = selaura::get<selaura::feature_manager>();
        int index = 0;

        feature_manager.for_each([&](auto& ft) {
            int row = index / cols;
            int col = index % cols;
            float x = content_x + (col * (card_width + padding));
            float y = content_y + (row * (card_height + padding));
            index++;
        });
    }

    void clickgui::on_input(input_event& ev) {

    }
};