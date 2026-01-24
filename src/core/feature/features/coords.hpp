#pragma once
#include <pch.hpp>

#include "../feature.hpp"

namespace selaura {
    struct coords : feature {
        coords() {}

        void on_enable() override;
        void on_disable() override;

        void on_render(render_event& ev);
        void on_mcupdate(mcgame_update& ev);

        std::string_view get_name() const override { return name; }
        static constexpr hat::fixed_string name = "Custom Coords";
    private:
        glm::vec3 pos = {0, 0, 0};
    };
};