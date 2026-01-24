#pragma once
#include <pch.hpp>

#include "ecs/EntityId.hpp"
#include "ecs/EntityRegistry.hpp"
#include "ecs/IEntityComponent.hpp"

template<std::derived_from<IEntityComponent> Type>
struct entt::component_traits<Type, EntityId> {
    using element_type = Type;
    using entity_type = EntityId;
    static constexpr bool in_place_delete = true;
    static constexpr std::size_t page_size = 128 * !std::is_empty_v<Type>;
};

template<typename Type>
struct entt::storage_type<Type, EntityId> {
    using type = basic_storage<Type, EntityId>;
};

class EntityContext {
public:
    EntityRegistry& mRegistry;
    entt::basic_registry<EntityId>& mEnttRegistry;
    const EntityId mEntity;

    template <std::derived_from<IEntityComponent> T>
    [[nodiscard]] decltype(auto) tryGetComponent(this auto&& self) {
        return self.mEnttRegistry.template try_get<T>(self.mEntity);
    }

    template <std::derived_from<IEntityComponent> T>
    [[nodiscard]] bool hasComponent(this const auto& self) {
        return self.mEnttRegistry.template all_of<T>(self.mEntity);
    }

    template <std::derived_from<IEntityComponent> T>
    decltype(auto) getOrAddComponent(this auto&& self) {
        return self.mEnttRegistry.template get_or_emplace<T>(self.mEntity);
    }

    template <std::derived_from<IEntityComponent> T>
    void removeComponent(this auto&& self) {
        self.mEnttRegistry.template remove<T>(self.mEntity);
    }

    template <std::derived_from<IEntityComponent> T>
    [[nodiscard]] decltype(auto) getComponent(this auto&& self) {
        ENTT_ASSERT(
            self.mEnttRegistry.template all_of<T>(self.mEntity),
            "Component missing at runtime."
        );

        return self.mEnttRegistry.template get<T>(self.mEntity);
    }
};