#pragma once
#include <pch.hpp>
#include "components/IEntityComponent.hpp"

class Actor {
public:
    EntityContext& getEntityContext() {
        return hat::member_at<EntityContext>(this, 0x8);
    }
};