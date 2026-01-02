#include "sui.hpp"
using namespace selaura;

LOAD_RESOURCE(selaura_icon_png)
LOAD_RESOURCE(selaura_logo_png)

void sui::panel(float x, float y, float width, float height, float radius, panel_types type) {
    switch (type) {
    case panel_types::background:
        selaura::renderer->draw_filled_rect(x, y, width, height, radius, {34, 34, 34, 255});
        break;
    case panel_types::raised:
        selaura::renderer->draw_filled_rect(x, y, width, height, {15, 0, 0, 15}, {24, 24, 24, 255});
        break;
    }
}

void sui::logo(float x, float y, float size) {
    selaura::renderer->draw_image(GET_RESOURCE(selaura_icon_png), x, y, size, size);
}

void sui::text(float x, float y, float size, std::string_view text, glm::vec4 color) {
    selaura::renderer->draw_text(text, x, y, size, color);
}