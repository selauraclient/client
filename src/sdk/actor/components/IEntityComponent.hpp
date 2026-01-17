#pragma once
#include <pch.hpp>

struct IEntityComponent {};

struct EntityId {
    std::uint32_t rawId;
    [[nodiscard]] constexpr bool operator==(const EntityId& other) const = default;
    [[nodiscard]] constexpr operator std::uint32_t() const {
        return rawId;
    }
};

struct EntityIdTraits {
    using value_type = EntityId;

    using entity_type = std::uint32_t;
    using version_type = std::uint16_t;

    static constexpr entity_type entity_mask = 0x3FFFF;
    static constexpr entity_type version_mask = 0x3FFF;
};

template<>
struct entt::entt_traits<EntityId> : entt::basic_entt_traits<EntityIdTraits> {
    static constexpr std::size_t page_size = 2048;
};

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

template<std::derived_from<IEntityComponent> Type>
struct entt::type_hash<Type> {
    [[nodiscard]] static consteval id_type value() noexcept {
        constexpr auto name = Type::type_name;
        return hashed_string::value(name.data(), name.size());
    }

    [[nodiscard]] consteval operator id_type() const noexcept {
        return value();
    }
};

struct EntityRegistry : std::enable_shared_from_this<EntityRegistry> {
    std::string mDebugName;
    entt::basic_registry<EntityId> mRegistry;
    unsigned int mId;
    std::function<void(EntityId)> mPreEntityInvoke;
    std::function<void(EntityId)> mPostEntityInvoke;
};

struct EntityContext {
    EntityRegistry& mRegistry;
    entt::basic_registry<EntityId>& mEnttRegistry;
    const EntityId mEntity;

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
};