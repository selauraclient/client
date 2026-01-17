#pragma once
#include <pch.hpp>

#include "../feature.hpp"

namespace selaura {
    struct coords : feature {
        coords() {}

        void on_enable() override;
        void on_disable() override;

        void on_render(render_event& ev) override;
        void on_mcupdate(mcgame_update& ev) override;
        void on_input(input_event& ev) override {};
    private:
        glm::vec3 pos = {0, 0, 0};
    };
};