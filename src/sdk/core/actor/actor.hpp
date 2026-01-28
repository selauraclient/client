#pragma once
#include <pch.hpp>
#include <sdk/bedrock/actor/Actor.hpp>
#include <sdk/bedrock/actor/components/StateVectorComponent.hpp>
#include <sdk/bedrock/actor/components/OffsetsComponent.hpp>

namespace selaura::sdk {
    struct selaura_actor {
        explicit selaura_actor(Actor* actor) : m_actor(actor, [](Actor*) {}) {}

        template <class T>
        requires std::derived_from<T, Actor>
        [[nodiscard]] T* as(this auto& self) {
            return static_cast<T*>(self.m_actor.get());
        }

        [[nodiscard]] glm::vec3 get_position(bool correct = false) const;
    private:
        std::shared_ptr<Actor> m_actor;
    };
};
