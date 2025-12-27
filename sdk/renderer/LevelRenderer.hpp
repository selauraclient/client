#pragma once
#include <pch.hpp>

class ScreenContext;
class FrameRenderObject;

class LevelRenderer {
public:
    MCAPI void renderLevel(ScreenContext& screenContext, FrameRenderObject const& renderObj);
};