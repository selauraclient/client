#pragma once
#include <pch.hpp>

class NameTagRenderObjectCollection;
class ScreenContext;

class LevelRendererPlayer {
public:
    MCAPI NameTagRenderObjectCollection extractNameTags(ScreenContext& screenContext);

    // thunk for referencing localPlayer check in extractNameTags
    MCAPI static void $extractNameTags_localPlayerCheck();
};