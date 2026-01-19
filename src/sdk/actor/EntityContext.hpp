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
    template <std::derived_from<IEntityComponent> T>
    [[nodiscard]] T* tryGetComponent() {
        return this->mEnttRegistry.try_get<T>(this->mEntity);
    }

    template <std::derived_from<IEntityComponent> T>
    [[nodiscard]] const T* tryGetComponent() const {
        return this->mEnttRegistry.try_get<T>(this->mEntity);
    }

    template <std::derived_from<IEntityComponent> T>
    [[nodiscard]] bool hasComponent() const {
        return this->mEnttRegistry.all_of<T>(this->mEntity);
    }

    template <std::derived_from<IEntityComponent> T>
    T& getOrAddComponent() {
        return this->mEnttRegistry.get_or_emplace<T>(this->mEntity);
    }

    template <std::derived_from<IEntityComponent> T>
    void removeComponent() {
        this->mEnttRegistry.remove<T>(this->mEntity);
    }

    EntityRegistry& mRegistry;
    entt::basic_registry<EntityId>& mEnttRegistry;
    const EntityId mEntity;
};