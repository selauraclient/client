#include "console.hpp"

void selaura::console_impl::render() {
    ImGui::SetNextWindowSize(ImVec2(500.0f, 200.0f), ImGuiCond_Once);
    if (!ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::End();
        return;
    }

    ImGui::BeginChild(
        "ScrollingRegion",
        ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
        false,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    {
        std::lock_guard<std::mutex> lock(message_mutex);
        for (const auto& msg : message_history) {
            ImGui::PushStyleColor(ImGuiCol_Text, msg.color);
            ImGui::TextUnformatted(msg.text.c_str());
            ImGui::PopStyleColor();
        }
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::Separator();

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    bool submit = ImGui::InputText(
        "##ConsoleInput",
        input_buf,
        IM_ARRAYSIZE(input_buf),
        ImGuiInputTextFlags_EnterReturnsTrue
    );

    if (submit && input_buf[0] != '\0') {
        push_text(input_buf);
        input_buf[0] = '\0';
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();
}

void selaura::console_impl::push_text(const std::string& text, const ImVec4& color) {
    std::lock_guard<std::mutex> lock(message_mutex);
    this->message_history.push_back({ text, color });
}

void selaura::console_impl::shutdown() {
    std::lock_guard<std::mutex> lock(message_mutex);
    message_history.clear();
    cmd_history.clear();
    input_buf[0] = '\0';
}