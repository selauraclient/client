#pragma once
#include <pch.hpp>

class MinecraftGame {
public:
    using client_instances_type = std::map<SubClientId, std::shared_ptr<ClientInstance>>;
    client_instances_type $getSwapChain() {
        return hat::member_at<client_instances_type>(this, 0x9D0);
    }

    MCAPI void _update();
};