#pragma once
#include <pch.hpp>

class Frame;
class ClearQuad;
class TextVideoMemBlitter;

namespace bgfx {
    namespace d3d11 {
        class RenderContextD3D11 {
        public:
            std::byte pad[0x228];
            IDXGISwapChain* mSwapChain;

            MCAPI void submit(Frame* _render, ClearQuad& _clearQuad, TextVideoMemBlitter& _textVideoMemBlitter);
        };
    };
};