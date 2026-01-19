#pragma once
#include <pch.hpp>
#include "EntityId.hpp"

struct EntityRegistry : std::enable_shared_from_this<EntityRegistry> {
    std::string mDebugName;
    entt::basic_registry<EntityId> mRegistry;
    unsigned int mId;
    std::function<void(EntityId)> mPreEntityInvoke;
    std::function<void(EntityId)> mPostEntityInvoke;
};