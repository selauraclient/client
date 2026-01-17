#pragma once
#include <pch.hpp>
#include "IEntityComponent.hpp"

struct OffsetsComponent : IEntityComponent {
    static constexpr hat::fixed_string type_name = "struct OffsetsComponent";

    float mHeightOffset;
    float mExplosionOffset;
    glm::vec3 mHeadOffset;
    glm::vec3 mDropOffset;
    glm::vec3 mEyeOffset;
    glm::vec3 mMouthOffset;
    glm::vec3 mBreathingOffset;
};