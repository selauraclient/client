#pragma once
#include <pch.hpp>

#include <sdk/actor/LocalPlayer.hpp>

class ClientInstance {
public:
    LocalPlayer* getLocalPlayer() {
        return hat::member_at<LocalPlayer*>(this, 0x1F);
    }
};