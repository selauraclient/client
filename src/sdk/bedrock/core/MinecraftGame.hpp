#pragma once
#include <pch.hpp>

#include <sdk/bedrock/core/SubClientId.hpp>
#include <sdk/bedrock/core/ClientInstance.hpp>

class MinecraftGame {
public:
    using ClientInstanceMap_t = std::map<SubClientId, std::shared_ptr<ClientInstance>>;
    const ClientInstanceMap_t& getClientInstanceMap() {
        return hat::member_at<ClientInstanceMap_t>(this, 0x9D0);
    }

    std::shared_ptr<ClientInstance> getPrimaryClientInstance() {
        const auto& map = this->getClientInstanceMap();
        return map.at(SubClientId::PrimaryClient);
    }

    bool isPrimaryClientInstanceReady() {
        const auto& map = this->getClientInstanceMap();

        if (map.empty()) return false;

        auto it = map.begin();
        return it->first == SubClientId::PrimaryClient;
    }

    LocalPlayer* getPrimaryLocalPlayer() {
        if (!this->isPrimaryClientInstanceReady()) return nullptr;
        auto ci = this->getPrimaryClientInstance();
        return ci->getLocalPlayer();
    }

    GuiData* getPrimaryGuiData() {
        if (!this->isPrimaryClientInstanceReady()) return nullptr;
        auto ci = this->getPrimaryClientInstance();
        return ci->getGuiData();
    }

    MCAPI void _update();
    MCAPI void onDeviceLost();
};