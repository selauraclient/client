#pragma once
#include "../ecs/IEntityComponent.hpp"
#include "MoveInputState.hpp"

struct MoveInputComponent : public IEntityComponent {
    enum class Flag : int {
        Sneaking = 0,
        Sprinting = 1,
        WantUp = 2,
        WantDown = 3,
        Jumping = 4,
        AutoJumpingInWater = 5,
        MoveInputStateLocked = 6,
        PersistSneak = 7,
        AutoJumpEnabled = 8,
        IsCameraRelativeMovementEnabled = 9,
        IsRotControlledByMoveDirection = 10,
        Count = 11,
    };

    static constexpr hat::fixed_string type_name = "struct MoveInputComponent";

    MoveInputState mInputState;
    MoveInputState mRawInputState;
    unsigned char mHoldAutoJumpInWaterTicks;
    glm::vec2 mMove;
    glm::vec2 mLookDelta;
    glm::vec2 mInteractDir;
    glm::vec3 mDisplacement;
    glm::vec3 mDisplacementDelta;
    glm::vec3 mCameraOrientation;
    std::bitset<11> mFlagValues;
    std::array<bool, 2> mIsPaddling;
};
