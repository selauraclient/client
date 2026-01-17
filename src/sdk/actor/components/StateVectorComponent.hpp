#pragma once
#include <pch.hpp>

#include "IEntityComponent.hpp"

struct StateVectorComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct StateVectorComponent";

    glm::vec3 mPos;
    glm::vec3 mPosPrev;
    glm::vec3 mPosDelta;
};