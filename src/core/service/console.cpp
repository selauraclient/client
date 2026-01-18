#include "console.hpp"

void selaura::console::render() {

}

void selaura::console::push_text(const std::string& text, const glm::vec4& color) {
    std::lock_guard<std::mutex> lock(message_mutex);
    this->message_history.push_back({ text, color });
}

void selaura::console::shutdown() {
    std::lock_guard<std::mutex> lock(message_mutex);
    message_history.clear();
    cmd_history.clear();
    input_buf[0] = '\0';
}