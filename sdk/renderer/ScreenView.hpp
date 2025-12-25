#pragma once
#include <pch.hpp>

class UIRenderContext;

class ScreenView {
public:
    MCAPI void setupAndRender(UIRenderContext& ctx);
};