#include "screen.hpp"

namespace selaura {
    void screen::set_enabled(bool state) {
        if (enabled == state) return;
        enabled = state;

        if (enabled) {
            on_enable();
        } else {
            on_disable();
        }
    }

    bool screen::get_enabled() {
        return enabled;
    }

    void screen::toggle() {
        set_enabled(!enabled);
    }

    void screen::set_key(uint32_t new_key) {
        this->key = new_key;
    }

    uint32_t screen::get_key() {
        return this->key;
    }
};