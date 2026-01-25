#pragma once
#include <pch.hpp>

#include "sgfx.hpp"
#include "core/event/event.hpp"

namespace sgfx::ui {
    inline selaura::input_event current_input{};

    void input_handler(const selaura::input_event& ev);
    void button(glm::vec2 pos, glm::vec2 size, const std::string& text, float font_size, std::function<void()> callback);
};