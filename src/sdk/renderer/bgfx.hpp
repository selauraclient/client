#pragma once
#include <pch.hpp>

class Frame;
class ClearQuad;
class TextVideoMemBlitter;

namespace bgfx {
    namespace d3d11 {
        class RendererContextD3D11 {
        public:
            IDXGISwapChain* $getSwapChain() {
                return hat::member_at<IDXGISwapChain*>(this, 0x228);
            }

            MCAPI bool isDeviceRemoved();
            MCAPI void submit(Frame* _render, ClearQuad& _clearQuad, TextVideoMemBlitter& _textVideoMemBlitter);
        };
    };

    namespace d3d12 {
        class RendererContextD3D12 {
        public:
            IDXGISwapChain* $getSwapChain() {
                return hat::member_at<IDXGISwapChain*>(this, 0x308);
            }

            MCAPI bool isDeviceRemoved();
            MCAPI void submit(Frame* _render, ClearQuad& _clearQuad, TextVideoMemBlitter& _textVideoMemBlitter);
        };
    };
};