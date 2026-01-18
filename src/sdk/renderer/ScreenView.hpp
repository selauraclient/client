#pragma once
#include <pch.hpp>
#include "gui/VisualTree.hpp"

class UIRenderContext;

class ScreenView {
public:
    VisualTree* getVisualTree() {
        return hat::member_at<VisualTree*>(this, 0x48);
    }

    MCAPI void render(UIRenderContext& ctx);
};