#pragma once
#include <pch.hpp>

namespace selaura {
    struct event {
        virtual ~event() = default;
        bool cancelled = false;
    };

    struct mcgame_update final : event {};
    struct render_event final : event {
        IDXGISwapChain* swapChain;
        float screenWidth;
        float screenHeight;
    };
};