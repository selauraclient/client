#pragma once
#include <pch.hpp>

namespace selaura {
    struct sui {
        enum class panel_types : int {
            background = 0,
            raised = 1
        };
        static void panel(float x, float y, float width, float height, float radius = 0.0f, panel_types type = panel_types::background);
        static void logo(float x, float y, float width);
        static void text(float x, float y, float size, std::string_view text, glm::vec4 color = {255, 255, 255, 255});
    };
};