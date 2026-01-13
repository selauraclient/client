#include "renderer_d3d.hpp"

namespace sgfx {
    glm::vec2 renderer_d3d::get_screen_size() const {
        DXGI_SWAP_CHAIN_DESC desc;
        swapchain->GetDesc(&desc);

        return {
            desc.BufferDesc.Width,
            desc.BufferDesc.Height,
        };
    }
}