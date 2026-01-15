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
        selaura::get<selaura::event_manager>().subscribe(this, &clickgui::on_input);
    }

    void clickgui::on_disable() {
        selaura::get<selaura::event_manager>().unsubscribe(this, &clickgui::on_render);
        selaura::get<selaura::event_manager>().unsubscribe(this, &clickgui::on_input);

        static ClientInstance* ci = nullptr;
        if (ci == nullptr) ci = selaura::mc->getPrimaryClientInstance().get();
        ci->grabCursor();
    }

    void clickgui::on_render(render_event& ev) {
        sgfx::draw_blur(0, 0, ev.screen_width, ev.screen_height, 2.f, 4);

        float menu_width = ev.screen_width * 0.55;
        float menu_height = ev.screen_height * 0.65;

        float menu_x = (ev.screen_width - menu_width) / 2;
        float menu_y = (ev.screen_height - menu_height) / 2;

        float sidebar_width = menu_width * 0.27;

        sgfx::draw_rect(menu_x, menu_y, menu_width, menu_height, {34, 34, 34, 1.f}, {15, 15, 15, 15});
        sgfx::draw_rect(menu_x, menu_y, sidebar_width, menu_height, {24, 24, 24, 1.f}, {15, 0, 0, 15});
    }

    void clickgui::on_input(input_event& ev) {
        static ClientInstance* ci = nullptr;
        if (ci == nullptr) ci = selaura::mc->getPrimaryClientInstance().get();
        ci->releaseCursor();
    }
};