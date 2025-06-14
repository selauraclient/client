#include "click_gui.hpp"

#include "../../instance.hpp"
#include "../../event/event_manager.hpp"
#include "../../renderer/renderer.hpp"
#include "../../sdk/globals.hpp"

namespace selaura {
    click_gui::click_gui() : screen() {
        this->set_hotkey(selaura::key::L); // L
        this->set_enabled(false);
    }

    void click_gui::on_render(selaura::setupandrender_event& ev) {
        auto currentScreen = ev.screen_view->getVisualTree()->getRoot()->getLayerName();
        static auto lastScreen = currentScreen;
        if (currentScreen != "hud_screen") {
            if (lastScreen != "hud_screen" || (currentScreen != "toast_screen" && currentScreen != "debug_screen")) {
                lastScreen = currentScreen;
                this->set_enabled(false);
                return;
            }
        } else {
            lastScreen = "hud_screen";
        }

        auto& io = ImGui::GetIO();

        glm::vec2 mod_menu_size = { io.DisplaySize.x * 0.5 , io.DisplaySize.x * 0.3 };
        glm::vec2 mod_menu_pos = { (io.DisplaySize.x - mod_menu_size.x) / 2 , (io.DisplaySize.y - mod_menu_size.y) / 2 };
        glm::vec2 sidebar_size = { mod_menu_size.x * 0.3, mod_menu_size.y };
        glm::vec2 bottom_bar_size { sidebar_size.x, sidebar_size.y * 0.1 };
        glm::vec2 bottom_bar_pos = { mod_menu_pos.x, mod_menu_pos.y + sidebar_size.y - bottom_bar_size.y };

        ev.renderer.draw_filled_rect({ mod_menu_pos.x - 2, mod_menu_pos.y - 2 }, { mod_menu_size.x + 4, mod_menu_size.y + 4 }, { 68, 68, 68, 255 }, 15.0f);
        ev.renderer.draw_filled_rect(mod_menu_pos, mod_menu_size, {34, 34, 34, 255}, 15.0f);
        ev.renderer.draw_filled_rect(mod_menu_pos, sidebar_size, {24, 24, 24, 255}, 15.0f, ImDrawFlags_RoundCornersLeft);
        ev.renderer.draw_filled_rect({ mod_menu_pos.x + sidebar_size.x, mod_menu_pos.y }, { 2, mod_menu_size.y }, { 68, 68, 68, 255 }, 0.f);
        ev.renderer.draw_filled_rect(bottom_bar_pos, {sidebar_size.x, 2}, {68, 68, 68, 255}, 0.f);
    }
};