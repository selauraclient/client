#include "feature.hpp"

namespace selaura {
    void feature::set_enabled(bool state) {
        if (enabled == state) return;
        enabled = state;

        if (enabled) {
            on_enable();
        } else {
            on_disable();
        }
    }

    bool feature::get_enabled() {
        return enabled;
    }

    void feature::toggle() {
        set_enabled(!enabled);
    }

    void feature::set_key(uint32_t new_key) {
        this->key = new_key;
    }

    uint32_t feature::get_key() {
        return this->key;
    }
};