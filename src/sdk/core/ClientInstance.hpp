#pragma once
#include <pch.hpp>

#include <sdk/actor/LocalPlayer.hpp>
#include <sdk/renderer/gui/GuiData.hpp>

#include "memory/memory.hpp"

class ClientInstance {
public:
    LocalPlayer* getLocalPlayer() {
        return selaura::call_vfunc<LocalPlayer*>(this, 0x1F);
    }

    GuiData* getGuiData() {
        return hat::member_at<GuiData*>(this, 0x648);
    }

    MCAPI void grabCursor();
    MCAPI void releaseCursor();
};
