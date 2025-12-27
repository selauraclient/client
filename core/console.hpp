#pragma once
#include <pch.hpp>

namespace selaura {
    class console_impl {
    private:
        std::vector<std::string> cmd_history;
        int cmd_history_pos = 0;

        std::vector<std::string> message_history;
        std::mutex message_mutex;

        char input_buf[256]{};
    public:
        void render();
        void push_text(const std::string& text);
        void shutdown();

        ~console_impl() {
            this->shutdown();
        }
    };

    inline std::unique_ptr<console_impl> console;
};
