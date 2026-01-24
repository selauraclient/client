#pragma once
#include <pch.hpp>
#include <core/event/event.hpp>

namespace selaura {
    struct feature {
        virtual ~feature() = default;

        virtual void on_enable() = 0;
        virtual void on_disable() = 0;
        void set_enabled(bool state);
        bool get_enabled();

        void toggle();

        void set_key(uint32_t new_key);
        uint32_t get_key();

        [[nodiscard]] virtual std::string_view get_name() const { return name; }
        static constexpr hat::fixed_string name = "Unknown Mod";
    private:
        bool enabled = false;
        uint32_t key = 0;
    };
};