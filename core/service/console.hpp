#pragma once
#include <pch.hpp>

namespace selaura {
    struct log_message {
        std::string text;
        ImVec4 color;
    };

    class console {
    private:
        std::vector<std::string> cmd_history;
        int cmd_history_pos = 0;

        std::vector<log_message> message_history;
        std::mutex message_mutex;

        char input_buf[256]{};
    public:
        void render();
        void push_text(const std::string& text, const ImVec4& color = ImVec4(0.95f, 0.95f, 0.97f, 1.00f));
        void shutdown();

        ~console() {
            this->shutdown();
        }
    };
};