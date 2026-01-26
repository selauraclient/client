#pragma once
#include <pch.hpp>

#include "../feature.hpp"

namespace selaura {
    struct third_person_nametag : feature {
        third_person_nametag() {}

        void on_enable() override;
        void on_disable() override;

        std::string_view get_name() const override { return name; }
        static constexpr hat::fixed_string name = "Third Person Nametag";
    private:
        static constexpr std::size_t size = 6;
        std::array<std::byte, size> original_bytes{};
        void* ptr = nullptr;
    };
};