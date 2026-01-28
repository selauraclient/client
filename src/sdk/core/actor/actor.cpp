#include "actor.hpp"

namespace selaura::sdk {
    glm::vec3 selaura_actor::get_position(bool correct) const {
        glm::vec3 result = {0, 0, 0};
        auto& ent = m_actor->getEntityContext();
        result = ent.tryGetComponent<StateVectorComponent>()->mPos;

        if (correct) {
            result -= ent.tryGetComponent<OffsetsComponent>()->mHeightOffset;
        }

        return result;
    }
};