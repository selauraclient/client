#pragma once
#include <pch.hpp>

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