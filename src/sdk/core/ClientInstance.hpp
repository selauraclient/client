#pragma once
#include <pch.hpp>

#include <sdk/actor/LocalPlayer.hpp>

#include "memory/memory.hpp"

class ClientInstance {
public:
    LocalPlayer* getLocalPlayer() {
        return selaura::call_vfunc<LocalPlayer*>(this, 0x1F);
    }

    MCAPI void grabCursor();
    MCAPI void releaseCursor();
};
