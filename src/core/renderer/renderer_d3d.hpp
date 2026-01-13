#pragma once
#include <pch.hpp>
#include "sgfx.hpp"

namespace sgfx {
    struct renderer_d3d : public backend_interface {
        [[nodiscard]] virtual glm::vec2 get_screen_size() const override;
    protected:
        winrt::com_ptr<IDXGISwapChain3> swapchain;
    };
};