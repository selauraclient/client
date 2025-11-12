#pragma once
#include <pch.hpp>
class IEntityComponent;

template <std::derived_from<IEntityComponent> Type>
struct entt::type_hash<Type> {
    [[nodiscard]] static consteval id_type value() noexcept {
        constexpr auto name = Type::type_name;
        return hashed_string::value(name.data(), name.size());
    }

    [[nodiscard]] consteval operator id_type() const noexcept {
        return value();
    }
};