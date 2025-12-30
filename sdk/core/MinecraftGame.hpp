#pragma once
#include <pch.hpp>

class MinecraftGame {
public:
    std::byte pad[0x9D0];
    std::map<SubClientId, std::shared_ptr<ClientInstance>> mClientInstances;

    MCAPI void _update();
};