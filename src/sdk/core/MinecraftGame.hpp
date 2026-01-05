#pragma once
#include <pch.hpp>

#include <sdk/core/SubClientId.hpp>
#include <sdk/core/ClientInstance.hpp>

class MinecraftGame {
public:
    using client_instances_type = std::map<SubClientId, std::shared_ptr<ClientInstance>>;
    client_instances_type $getSwapChain() {
        return hat::member_at<client_instances_type>(this, 0x9D0);
    }

    MCAPI void _update();
};